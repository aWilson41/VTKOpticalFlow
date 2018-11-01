#include "vtkImagePyramid.h"
#include <algorithm>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkImageResize.h>
//#include <chrono>

vtkStandardNewMacro(vtkImagePyramid);

void vtkImagePyramid::SetNumImages(unsigned int numImages)
{
	NumImages = numImages;
	outputs.resize(NumImages);
	Modified();
}

int vtkImagePyramid::RequestData(vtkInformation* vtkNotUsed(request), vtkInformationVector** inputVec, vtkInformationVector* outputVec)
{
	// Get input image
	vtkInformation* inInfo = inputVec[0]->GetInformationObject(0);
	vtkImageData* input = vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
	int* dim = input->GetDimensions();

	// Get output
	vtkInformation* outInfo = outputVec->GetInformationObject(0);
	vtkImageData* output = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
	output->DeepCopy(input);

	double ds = 0.0;
	if (NumImages > 1)
		ds = (ScaleRange[1] - ScaleRange[0]) / (NumImages - 1);
	double scale = ScaleRange[1];
	for (unsigned int i = 0; i < NumImages; i++)
	{
		vtkSmartPointer<vtkImageResize> resize = vtkSmartPointer<vtkImageResize>::New();
		resize->SetInputData(input);
		resize->SetResizeMethodToOutputDimensions();
		resize->SetOutputDimensions(scale * dim[0], scale * dim[1], scale * dim[2]);
		resize->Update();
		outputs[i] = resize->GetOutput();
		scale -= ds;
	}

	return 1;
}