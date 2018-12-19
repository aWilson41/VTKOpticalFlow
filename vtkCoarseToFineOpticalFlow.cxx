#include "vtkCoarseToFineOpticalFlow.h"
#include "vtkImagePyramid.h"
#include "vtkImageOpticalFlow.h"
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkCoarseToFineOpticalFlow);

vtkCoarseToFineOpticalFlow::vtkCoarseToFineOpticalFlow()
{
	this->SetNumberOfInputPorts(2);
	this->SetNumberOfOutputPorts(1);
}

static const int calcIndex(int x, int y, int z, int width, int height) { return x + width * (y + height * z); }

static float trilinearSamplePoint(vtkImageData* imageData, float x, float y, float z, int comp)
{
	float* imagePtr = static_cast<float*>(imageData->GetScalarPointer());
	double* spacing = imageData->GetSpacing();
	double* origin = imageData->GetOrigin();
	int* dim = imageData->GetDimensions();

	// We assume point x, y, z is in the image
	int xi = (x - origin[0]) / spacing[0];
	int yi = (y - origin[1]) / spacing[1];
	int zi = (z - origin[2]) / spacing[2];

	// Get the intensities at the 8 nearest neighbors
	float i000 = imagePtr[calcIndex(xi, yi, zi, dim[0], dim[1]) * 3 + comp];
	float i100 = imagePtr[calcIndex(xi + 1, yi, zi, dim[0], dim[1]) * 3 + comp];
	float i110 = imagePtr[calcIndex(xi + 1, yi + 1, zi, dim[0], dim[1]) * 3 + comp];
	float i010 = imagePtr[calcIndex(xi, yi + 1, zi, dim[0], dim[1]) * 3 + comp];

	float i001 = imagePtr[calcIndex(xi, yi, zi + 1, dim[0], dim[1]) * 3 + comp];
	float i101 = imagePtr[calcIndex(xi + 1, yi, zi + 1, dim[0], dim[1]) * 3 + comp];
	float i111 = imagePtr[calcIndex(xi + 1, yi + 1, zi + 1, dim[0], dim[1]) * 3 + comp];
	float i011 = imagePtr[calcIndex(xi, yi + 1, zi + 1, dim[0], dim[1]) * 3 + comp];

	// Get the fractional/unit distance from nearest neighbor 000
	float rx = xi * spacing[0] + origin[0]; // Position of node
	rx = (x - rx) / spacing[0]; // (Node - actual point position) / voxel width
	float ry = yi * spacing[1] + origin[1];
	ry = (y - ry) / spacing[1];
	float rz = zi * spacing[2] + origin[2];
	rz = (z - rz) / spacing[2];

	// Now we do the trilinear interpolation
	float ax = i000 + (i100 - i000) * rx;
	float bx = i010 + (i110 - i010) * rx;
	float cy = ax + (bx - ax) * ry;

	float dx = i001 + (i101 - i001) * rx;
	float ex = i011 + (i111 - i011) * rx;
	float fy = dx + (ex - dx) * ry;

	float gz = cy + (fy - cy) * rz;
	return gz;
}

int vtkCoarseToFineOpticalFlow::RequestData(vtkInformation* request, vtkInformationVector** inputVec, vtkInformationVector* outputVec)
{
	// Get input image
	vtkInformation* inInfo1 = inputVec[0]->GetInformationObject(0);
	vtkImageData* input1 = vtkImageData::SafeDownCast(inInfo1->Get(vtkDataObject::DATA_OBJECT()));
	int* dim1 = input1->GetDimensions();
	double* origin1 = input1->GetOrigin();
	double* spacing1 = input1->GetSpacing();

	vtkInformation* inInfo2 = inputVec[1]->GetInformationObject(0);
	vtkImageData* input2 = vtkImageData::SafeDownCast(inInfo2->Get(vtkDataObject::DATA_OBJECT()));

	vtkSmartPointer<vtkImagePyramid> imagePyr1 = vtkSmartPointer<vtkImagePyramid>::New();
	imagePyr1->SetInputData(input1);
	imagePyr1->SetNumImages(NumLevels);
	imagePyr1->SetScaleRange(ScaleRange[0], ScaleRange[1]);
	imagePyr1->Update();

	vtkSmartPointer<vtkImagePyramid> imagePyr2 = vtkSmartPointer<vtkImagePyramid>::New();
	imagePyr2->SetInputData(input2);
	imagePyr2->SetNumImages(NumLevels);
	imagePyr2->SetScaleRange(ScaleRange[0], ScaleRange[1]);
	imagePyr2->Update();

	// Initialize the output with the first level of optical flow
	vtkInformation* outInfo = outputVec->GetInformationObject(0);
	vtkImageData* output = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
	vtkSmartPointer<vtkImageOpticalFlow> optFlowFilterLevel1 = vtkSmartPointer<vtkImageOpticalFlow>::New();
	optFlowFilterLevel1->SetInputData(0, imagePyr1->GetOutputs()[0]);
	optFlowFilterLevel1->SetInputData(1, imagePyr2->GetOutputs()[0]);
	optFlowFilterLevel1->Update();
	output->DeepCopy(optFlowFilterLevel1->GetOutput());
	float* outPtr = static_cast<float*>(output->GetScalarPointer());

	// Fine to coarse
	float ds = 0.0f;
	if (NumLevels > 1)
		ds = (ScaleRange[1] - ScaleRange[0]) / (NumLevels - 1);
	int progress = 0;
	for (int i = 1; i < NumLevels; i++)
	{
		float invScale = 1.0f / (ScaleRange[1] - ds * i);

		// Calculate the next level of flow
		vtkSmartPointer<vtkImageOpticalFlow> optFlowFilter = vtkSmartPointer<vtkImageOpticalFlow>::New();
		optFlowFilter->SetInputData(0, imagePyr1->GetOutputs()[i]);
		optFlowFilter->SetInputData(1, imagePyr2->GetOutputs()[i]);
		optFlowFilter->Update();
		vtkImageData* optFlowImage = optFlowFilter->GetOutput();
		int* dim = imagePyr1->GetOutputs()[i]->GetDimensions();

		// Now resample the image into the larger image
		double ratio = static_cast<double>(dim[0] - 1) / dim1[0];
		int index = 0;
		// For every voxel of the base/init image we are sampling to
		for (int z = 0; z < dim1[2]; z++)
		{
			for (int y = 0; y < dim1[1]; y++)
			{
				for (int x = 0; x < dim1[0]; x++)
				{
					// Calculate the corresponding node in the coarser image
					float sXPos = (x * spacing1[0] + origin1[0]) * ratio;
					float sYPos = (y * spacing1[1] + origin1[1]) * ratio;
					float sZPos = (z * spacing1[2] + origin1[2]) * ratio;

					// trilinearly interpolate each component from the smaller image
					float flowX = trilinearSamplePoint(optFlowImage, sXPos, sYPos, sZPos, 0) * invScale;
					float flowY = trilinearSamplePoint(optFlowImage, sXPos, sYPos, sZPos, 1) * invScale;
					float flowZ = trilinearSamplePoint(optFlowImage, sXPos, sYPos, sZPos, 2) * invScale;
					outPtr[index++] = (outPtr[index] + flowX) * 0.5f;
					outPtr[index++] = (outPtr[index] + flowY) * 0.5f;
					outPtr[index++] = (outPtr[index] + flowZ) * 0.5f;
				}
			}
		}
	}

	return 1;
}