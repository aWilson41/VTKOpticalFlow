# VTKLKOpticalFlow
Lucas Kanade optical flow for 3D images in VTK. Also includes a coarse to fine extension of it and an example that glyphs the vectors for display.

VTKImageOpticalFlow: Computes the LK optical flow on an image for every voxel.
VTKCoarseToFineOpticalFlow: Computes the optical flow on multiple sizes of the image and interpolates. This allows us to capture large and small movement. Use SetNumLevels to set the number of images in the image pyramid and SetScaleRange to set the min and max scale. So if I choose ScaleRange = 0.25, 1.0 and 3 levels I get scale factors of 1.0, 0.5, 0.25.

![Alt text](https://andaharoo.files.wordpress.com/2018/10/screenshot.png)
