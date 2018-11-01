# VTKLKOpticalFlow
Lucas Kanade optical flow filter for 3D images in VTK. Also includes a coarse to fine extension of it and an example that glyphs the vectors for display.

VTKImageOpticalFlow: Computes the LK optical flow on an image for every voxel.

VTKCoarseToFineOpticalFlow: Computes the optical flow on multiple sizes of the image and interpolates. This allows us to capture large and small movement. Image sizes are determined by ScaleRange and NumLevels. For example, if I SetScaleRange(0.25, 1.0) and choose 3 levels I get scale factors 1.0, 0.625, and 0.25.

Currently only works and assumes float images. Will implement as vtkThreadImageAlgorithm soon.

![Alt text](https://andaharoo.files.wordpress.com/2018/10/screenshot.png)
