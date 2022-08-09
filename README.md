## Image Processing Tools
### Overview
This tool is a set of many simple and basic image processing algorithms that used to visualize these
operations based on OpenCV and Qt5 platform. When you want to preprocess some images 
or photos for subsequent further processing or build a deep network for CV or Image 
Process, you can use this simple tool to check out the effect of these operations 
on the images. Visualization these operations will help us better to learn these operations and 
make appropriate changes.

These are many image processing operations in this tool. For some complex functions, we 
use the library function in OpenCV library to implement them for better performance. In the meantime, 
we use the Qt5 to develop this tool for across-platforms and high performance.

### Language
- C++
- CMake

### Requirement
- C++ 14 (gcc/Clang/msvc 14.29)
- Qt-5.15.2 (build by msvc 14.29 x64)
- OpenCV-4.6.0 (build by msvc 14.29 x64)

### Image Processing Operations
1. Image Preprocessing: rgb to gray image, gray histogram generator, grayscale
equalization, gradient image, gradient sharpening and laplacian sharpening.
2. Noise Addition: add salt and pepper noise, add gaussian noise.
3. Edge Detection: roberts edge detection, sobel edge detection, prewitt operator, laplacian edge detection, 
canny edge detection and krisch edge detection.
4. Image Filter: window filtering, average filtering, median filtering, morphological filtering, gaussian filtering.
