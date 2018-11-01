#pragma once
#include <vector>
#include <vtkSmartPointer.h>
#include <vtkImageAlgorithm.h>
#include <vtkImageData.h>

class vtkImagePyramid : public vtkImageAlgorithm
{
public:
	static vtkImagePyramid* New();
	vtkTypeMacro(vtkImagePyramid, vtkImageAlgorithm);

	vtkImagePyramid() { }

	void SetInputData(vtkImageData* data) { vtkImageAlgorithm::SetInputData(data); }
	void SetNumImages(unsigned int numImages);
	vtkGetMacro(NumImages, unsigned int);
	void SetScaleRange(double min, double max)
	{
		ScaleRange[0] = min;
		ScaleRange[1] = max;
		Modified();
	}
	double* GetScaleRange() { return ScaleRange; }
	// Ordered fine to coarse
	std::vector<vtkSmartPointer<vtkImageData>> GetOutputs() { return outputs; }

protected:
	int RequestData(vtkInformation* request, vtkInformationVector** inputVec, vtkInformationVector* outputVec) VTK_OVERRIDE;

private:
	vtkImagePyramid(const vtkImagePyramid&);
	void operator=(const vtkImagePyramid&);

protected:
	unsigned int NumImages = 3;
	double ScaleRange[2];
	std::vector<vtkSmartPointer<vtkImageData>> outputs;
};