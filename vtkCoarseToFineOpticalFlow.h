#pragma once
#include <vtkImageAlgorithm.h>
#include <vtkSmartPointer.h>

class vtkCoarseToFineOpticalFlow : public vtkImageAlgorithm
{
public:
	static vtkCoarseToFineOpticalFlow* New();
	vtkTypeMacro(vtkCoarseToFineOpticalFlow, vtkImageAlgorithm);

	vtkSetMacro(NumLevels, int);
	vtkGetMacro(NumLevels, int);
	void SetScaleRange(float min, float max)
	{
		ScaleRange[0] = min;
		ScaleRange[1] = max;
		Modified();
	}
	double* GetScaleRange() { return ScaleRange; }

protected:
	vtkCoarseToFineOpticalFlow();
	~vtkCoarseToFineOpticalFlow() VTK_OVERRIDE { }

	int RequestData(vtkInformation* request, vtkInformationVector** inputVec, vtkInformationVector* outputVec) VTK_OVERRIDE;

	int NumLevels = 1;
	double ScaleRange[2] = { 0.25f, 1.0f };

private:
	vtkCoarseToFineOpticalFlow(const vtkCoarseToFineOpticalFlow&) VTK_DELETE_FUNCTION;
	void operator=(const vtkCoarseToFineOpticalFlow&) VTK_DELETE_FUNCTION;
};