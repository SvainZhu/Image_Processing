#ifndef IMAGE_PROCESSING_IMAGE_OPS_H
#define IMAGE_PROCESSING_IMAGE_OPS_H

#include <opencv2/opencv.hpp>

#include <string>

namespace imgproc {

enum class MorphType {
    Erode,
    Dilate,
    Open,
    Close,
    Gradient,
    TopHat,
    BlackHat
};

cv::Mat toGray(const cv::Mat &src);
cv::Mat resizeImage(const cv::Mat &src, int width, int height, bool keepAspect = false);
cv::Mat cropImage(const cv::Mat &src, const cv::Rect &roi);
cv::Mat rotateImage(const cv::Mat &src, double angle, double scale = 1.0,
                    const cv::Scalar &border = cv::Scalar());
cv::Mat flipImage(const cv::Mat &src, int flipCode);

cv::Mat histogramImage(const cv::Mat &src, int width = 512, int height = 300);
cv::Mat equalizeHistogram(const cv::Mat &src);
cv::Mat adjustBrightnessContrast(const cv::Mat &src, double alpha, double beta);
cv::Mat gammaCorrection(const cv::Mat &src, double gamma);

cv::Mat thresholdBinary(const cv::Mat &src, double thresholdValue, double maxValue = 255.0);
cv::Mat thresholdOtsu(const cv::Mat &src);
cv::Mat adaptiveThresholdImage(const cv::Mat &src, int blockSize = 11, double c = 2.0,
                               bool gaussian = true);

cv::Mat meanBlurImage(const cv::Mat &src, int kernelSize = 3);
cv::Mat gaussianBlurImage(const cv::Mat &src, int kernelSize = 5, double sigma = 0.0);
cv::Mat medianBlurImage(const cv::Mat &src, int kernelSize = 3);
cv::Mat bilateralFilterImage(const cv::Mat &src, int diameter = 9, double sigmaColor = 75.0,
                             double sigmaSpace = 75.0);

cv::Mat morphologyImage(const cv::Mat &src, MorphType type, int kernelSize = 3,
                        int iterations = 1);

cv::Mat cannyEdge(const cv::Mat &src, double lowThreshold = 80.0,
                  double highThreshold = 160.0, int apertureSize = 3);
cv::Mat sobelEdge(const cv::Mat &src, int dx = 1, int dy = 0, int kernelSize = 3);
cv::Mat laplacianEdge(const cv::Mat &src, int kernelSize = 3);

cv::Mat sharpen(const cv::Mat &src);
cv::Mat unsharpMask(const cv::Mat &src, double amount = 1.0, int kernelSize = 5,
                    double sigma = 1.0);

cv::Mat addSaltPepperNoise(const cv::Mat &src, double amount = 0.01, unsigned int seed = 0);
cv::Mat addGaussianNoise(const cv::Mat &src, double mean = 0.0, double stddev = 10.0,
                         unsigned int seed = 0);

cv::Mat convertColor(const cv::Mat &src, int code);
cv::Mat applyColorMapImage(const cv::Mat &src, int colorMap);
cv::Mat normalizeToByte(const cv::Mat &src);

MorphType parseMorphType(const std::string &name);
int parseColorMap(const std::string &name);
int parseColorConversion(const std::string &name);

}  // namespace imgproc

#endif  // IMAGE_PROCESSING_IMAGE_OPS_H
