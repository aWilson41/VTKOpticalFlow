#include "vtkImageOpticalFlow.h"
#include "vtkImagePyramid.h"
#include <vtkImageData.h>
#include <vtkImageResize.h>
#include <vtkImageSobel3D.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMatrix3x3.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkImageOpticalFlow);

vtkImageOpticalFlow::vtkImageOpticalFlow()
{
	this->SetNumberOfInputPorts(2);
	this->SetNumberOfOutputPorts(1);
}

static const int calcIndex(int x, int y, int z, int width, int height) { return x + width * (y + height * z); }

// Image 1, Image 2, Gradient Image 1, pos, dimensions
static const double* calcLucasKanada(float* inPtr1, float* inPtr2, double* gPtr1,
	int x, int y, int z, int width, int height, int depth)
{
	// Calculate window bounds
	int startZ = z - 2;
	if (startZ <= 0)
		startZ = 0;
	int startY = y - 2;
	if (startY <= 0)
		startY = 0;
	int startX = x - 2;
	if (startX <= 0)
		startX = 0;

	int endZ = startZ + 5;
	if (endZ > depth)
		endZ = depth;
	int endY = startY + 5;
	if (endY > height)
		endY = height;
	int endX = startX + 5;
	if (endX > width)
		endX = width;

	// Calculate least squares
	double sumXSqr, sumYSqr, sumZSqr, sumXY, sumYZ, sumXZ;
	sumXSqr = sumYSqr = sumZSqr = sumXY = sumYZ = sumXZ = 0.0;
	double sumXT, sumYT, sumZT;
	sumXT = sumYT = sumZT = 0.0;
	for (int z = startZ; z < endZ; z++)
	{
		for (int y = startY; y < endY; y++)
		{
			for (int x = startX; x < endX; x++)
			{
				// Index for both 1 and 3 component image
				int index = calcIndex(x, y, z, width, height);
				int index3 = index * 3;
				double gx1 = gPtr1[index3];
				double gy1 = gPtr1[index3 + 1];
				double gz1 = gPtr1[index3 + 2];

				sumXSqr += gx1 * gx1;
				sumYSqr += gy1 * gy1;
				sumZSqr += gz1 * gz1;
				sumXY += gx1 * gy1;
				sumYZ += gy1 * gz1;
				sumXZ += gx1 * gz1;

				double dI = inPtr2[index] - inPtr1[index];
				sumXT += dI * gx1;
				sumYT += dI * gy1;
				sumZT += dI * gz1;
			}
		}
	}

	// This gives us a 3 component least squares matrix (symmetric) ie: mat = A^T * A
	vtkMatrix3x3* mat = vtkMatrix3x3::New();
	double* matData = mat->GetData();
	matData[0] = sumXSqr;
	matData[4] = sumYSqr;
	matData[8] = sumZSqr;
	matData[1] = matData[3] = sumXY;
	matData[5] = matData[7] = sumYZ;
	matData[2] = matData[6] = sumXZ;
	mat->Invert();

	// A^T * b
	double* atb = new double[3]{ -sumXT, -sumYT, -sumZT };
	// Then we do (A^T * A) * (A^T * b) = [u, v, w]
	static double results[3];
	mat->MultiplyPoint(atb, results);

	return results;
}

int vtkImageOpticalFlow::RequestData(vtkInformation* request, vtkInformationVector** inputVec, vtkInformationVector* outputVec)
{
	// Get input image
	vtkInformation* inInfo1 = inputVec[0]->GetInformationObject(0);
	vtkImageData* input1 = vtkImageData::SafeDownCast(inInfo1->Get(vtkDataObject::DATA_OBJECT()));
	int* dim1 = input1->GetDimensions();
	int* extent1 = input1->GetExtent();
	double* spacing1 = input1->GetSpacing();
	double* origin1 = input1->GetOrigin();

	vtkInformation* inInfo2 = inputVec[1]->GetInformationObject(0);
	vtkImageData* input2 = vtkImageData::SafeDownCast(inInfo2->Get(vtkDataObject::DATA_OBJECT()));
	int* dim2 = input2->GetDimensions();
	int* extent2 = input2->GetExtent();
	double* spacing2 = input2->GetSpacing();
	double* origin2 = input2->GetOrigin();

	if (dim1[0] != dim2[0] || dim1[1] != dim2[1] || dim1[2] != dim2[2])
	{
		vtkWarningMacro(<< "Input's different dimensions");
		return 0;
	}

	float* inPtr1 = static_cast<float*>(input1->GetScalarPointer());
	float* inPtr2 = static_cast<float*>(input2->GetScalarPointer());

	// Allocate the output (should be the same or slightly smaller than the original)
	vtkInformation* outInfo = outputVec->GetInformationObject(0);
	vtkImageData* output = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
	output->SetExtent(extent1);
	output->SetOrigin(origin1);
	output->SetSpacing(spacing1);
	output->SetDimensions(dim1);
	output->SetNumberOfScalarComponents(3, outInfo);
	output->SetScalarType(VTK_FLOAT, outInfo);
	output->AllocateScalars(outInfo);
	float* outPtr = static_cast<float*>(output->GetScalarPointer());

	vtkSmartPointer<vtkImageSobel3D> g1Filter = vtkSmartPointer<vtkImageSobel3D>::New();
	g1Filter->SetInputData(input1);
	g1Filter->Update();
	double* g1Ptr = static_cast<double*>(g1Filter->GetOutput()->GetScalarPointer());
	int numPx = dim1[0] * dim1[1] * dim1[2];
	int index = 0;
	for (int z = 0; z < dim1[2]; z++)
	{
		for (int y = 0; y < dim1[1]; y++)
		{
			for (int x = 0; x < dim1[0]; x++)
			{
				// For every pixel we consider a 5x5x5 window
				const double* flow = calcLucasKanada(inPtr1, inPtr2, g1Ptr, x, y, z, dim1[0], dim1[1], dim1[2]);
				outPtr[index++] = flow[0];
				outPtr[index++] = flow[1];
				outPtr[index++] = flow[2];

				if (index % 10000 == 0)
					UpdateProgress(static_cast<float>(index) / numPx);
			}
		}
	}

	return 1;
}