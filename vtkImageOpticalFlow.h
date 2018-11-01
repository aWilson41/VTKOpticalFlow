#pragma once
#include <vtkImageAlgorithm.h>
#include <vtkSmartPointer.h>

class vtkImageOpticalFlow : public vtkImageAlgorithm
{
public:
	static vtkImageOpticalFlow* New();
	vtkTypeMacro(vtkImageOpticalFlow, vtkImageAlgorithm);

protected:
	vtkImageOpticalFlow();
	~vtkImageOpticalFlow() VTK_OVERRIDE { }

	int RequestData(vtkInformation* request, vtkInformationVector** inputVec, vtkInformationVector* outputVec) VTK_OVERRIDE;

private:
	vtkImageOpticalFlow(const vtkImageOpticalFlow&) VTK_DELETE_FUNCTION;
	void operator=(const vtkImageOpticalFlow&) VTK_DELETE_FUNCTION;
};