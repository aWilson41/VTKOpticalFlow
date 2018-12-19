#include "vtkCoarseToFineOpticalFlow.h"
#include <vtkActor.h>
#include <vtkArrowSource.h>
#include <vtkColorSeries.h>
#include <vtkColorTransferFunction.h>
#include <vtkGlyph3DMapper.h>
#include <vtkImageData.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkNIFTIReader.h>
#include <vtkNIFTIWriter.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>

#include <chrono>

// This is an example to demonstrate LK optical flow. Reads two nii images. Displays it. Writes it.
int main(int argc, char* argv[])
{
	std::string input1Str = "C:/Users/Andx_/Desktop/MH3D_25_Test1.nii";
	std::string input2Str = "C:/Users/Andx_/Desktop/MH3D_25_Test2.nii";
	std::string outputStr = "C:/Users/Andx_/Desktop/output.nii";
	double minScale = 0.6;
	double maxScale = 1.0;
	int numLevels = 3;
	double renderArrowScale = 0.3;

	vtkSmartPointer<vtkNIFTIReader> reader1 = vtkSmartPointer<vtkNIFTIReader>::New();
	reader1->SetFileName(input1Str.c_str());
	reader1->Update();
	vtkSmartPointer<vtkNIFTIReader> reader2 = vtkSmartPointer<vtkNIFTIReader>::New();
	reader2->SetFileName(input2Str.c_str());
	reader2->Update();

	auto start = std::chrono::steady_clock::now();

	// Computes flow from 1 -> 2. Outputs is 3 component float image
	vtkSmartPointer<vtkCoarseToFineOpticalFlow> filter = vtkSmartPointer<vtkCoarseToFineOpticalFlow>::New();
	filter->SetScaleRange(minScale, maxScale);
	filter->SetNumLevels(numLevels);
	filter->SetInputData(0, reader1->GetOutput());
	filter->SetInputData(1, reader2->GetOutput());
	filter->Update();

	auto end = std::chrono::steady_clock::now();
	printf("Time: %f\n", std::chrono::duration<double, std::milli>(end - start).count() / 1000.0);

	// Setup the vector field for rendering
	vtkSmartPointer<vtkColorSeries> colorSeries = vtkSmartPointer<vtkColorSeries>::New();
	colorSeries->SetColorScheme(vtkColorSeries::WARM);
	vtkSmartPointer<vtkColorTransferFunction> lut = vtkSmartPointer<vtkColorTransferFunction>::New();
	lut->SetColorSpaceToHSV();
	// Use a color series to create a transfer function
	int numColors = colorSeries->GetNumberOfColors();
	double* scalarRange = filter->GetOutput()->GetScalarRange();
	for (int i = 0; i < numColors; i++)
	{
		vtkColor3ub color = colorSeries->GetColor(i);
		double dColor[3];
		dColor[0] = static_cast<double> (color[0]) / 255.0;
		dColor[1] = static_cast<double> (color[1]) / 255.0;
		dColor[2] = static_cast<double> (color[2]) / 255.0;
		double t = scalarRange[0] + (scalarRange[1] - scalarRange[0])
			/ (numColors - 1) * i;
		lut->AddRGBPoint(t, dColor[0], dColor[1], dColor[2]);
	}

	vtkSmartPointer<vtkGlyph3DMapper> mapper = vtkSmartPointer<vtkGlyph3DMapper>::New();
	mapper->SetInputData(filter->GetOutput());
	vtkSmartPointer<vtkArrowSource> glyphSource = vtkSmartPointer<vtkArrowSource>::New();
	glyphSource->Update();
	mapper->SetSourceData(glyphSource->GetOutput());
	mapper->OrientOn();
	mapper->SetOrientationArray("ImageScalars");
	mapper->SetLookupTable(lut);
	mapper->SetScaleFactor(renderArrowScale);
	mapper->Update();
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	// Create a renderer, render window, and interactor
	vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renWindow = vtkSmartPointer<vtkRenderWindow>::New();
	renWindow->AddRenderer(ren);
	vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	iren->SetInteractorStyle(vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New());
	iren->SetRenderWindow(renWindow);

	// Add the actors to the scene
	ren->AddActor(actor);
	ren->SetBackground(0.55, 0.75, 0.62);

	// Render and interact
	renWindow->Render();
	iren->Start();

	vtkSmartPointer<vtkNIFTIWriter> writer = vtkSmartPointer<vtkNIFTIWriter>::New();
	writer->SetFileName(outputStr.c_str());
	writer->SetInputData(filter->GetOutput());
	writer->Update();

	return EXIT_SUCCESS;
}