#ifndef IMAGE_PROCESSING_IMAGE_OPS_H
#define IMAGE_PROCESSING_IMAGE_OPS_H

#include <opencv2/opencv.hpp>

#include <map>
#include <string>
#include <vector>

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
cv::Mat claheEqualize(const cv::Mat &src, double clipLimit = 2.0, int tileGridSize = 8);
cv::Mat adjustBrightnessContrast(const cv::Mat &src, double alpha, double beta);
cv::Mat gammaCorrection(const cv::Mat &src, double gamma);
cv::Mat grayWorldWhiteBalance(const cv::Mat &src);
cv::Mat adjustSaturation(const cv::Mat &src, double factor = 1.0);
cv::Mat adjustVibrance(const cv::Mat &src, double amount = 0.25);
cv::Mat adjustTemperatureTint(const cv::Mat &src, double temperature = 0.0,
                              double tint = 0.0);
cv::Mat applyToneCurve(const cv::Mat &src, const std::vector<cv::Point2f> &points,
                       const std::string &channel = "all");
cv::Mat applyPresetFilter(const cv::Mat &src, const std::string &preset,
                          double intensity = 1.0);
cv::Mat autoEnhance(const cv::Mat &src, double strength = 1.0);

cv::Mat thresholdBinary(const cv::Mat &src, double thresholdValue, double maxValue = 255.0);
cv::Mat thresholdOtsu(const cv::Mat &src);
cv::Mat adaptiveThresholdImage(const cv::Mat &src, int blockSize = 11, double c = 2.0,
                               bool gaussian = true);

cv::Mat meanBlurImage(const cv::Mat &src, int kernelSize = 3);
cv::Mat gaussianBlurImage(const cv::Mat &src, int kernelSize = 5, double sigma = 0.0);
cv::Mat medianBlurImage(const cv::Mat &src, int kernelSize = 3);
cv::Mat bilateralFilterImage(const cv::Mat &src, int diameter = 9, double sigmaColor = 75.0,
                             double sigmaSpace = 75.0);
cv::Mat denoiseNlMeans(const cv::Mat &src, float h = 10.0F, int templateWindowSize = 7,
                       int searchWindowSize = 21);

cv::Mat morphologyImage(const cv::Mat &src, MorphType type, int kernelSize = 3,
                        int iterations = 1);

cv::Mat cannyEdge(const cv::Mat &src, double lowThreshold = 80.0,
                  double highThreshold = 160.0, int apertureSize = 3);
cv::Mat autoCannyEdge(const cv::Mat &src, double sigma = 0.33, int apertureSize = 3);
cv::Mat sobelEdge(const cv::Mat &src, int dx = 1, int dy = 0, int kernelSize = 3);
cv::Mat laplacianEdge(const cv::Mat &src, int kernelSize = 3);
cv::Mat gradientMagnitude(const cv::Mat &src, int kernelSize = 3, bool useScharr = false);

cv::Mat contoursImage(const cv::Mat &src, double thresholdValue = 0.0, bool useOtsu = true,
                      int thickness = 2);
cv::Mat connectedComponentsImage(const cv::Mat &src, int minArea = 0);
cv::Mat distanceTransformImage(const cv::Mat &src);
cv::Mat watershedSegmentation(const cv::Mat &src);
cv::Mat houghLinesImage(const cv::Mat &src, double rho = 1.0, double theta = CV_PI / 180.0,
                        int threshold = 80, double minLineLength = 30.0,
                        double maxLineGap = 10.0);
cv::Mat houghCirclesImage(const cv::Mat &src, double dp = 1.2, double minDist = 40.0,
                          double param1 = 100.0, double param2 = 30.0,
                          int minRadius = 0, int maxRadius = 0);
cv::Mat templateMatchImage(const cv::Mat &src, const cv::Mat &templ, int method = cv::TM_CCOEFF_NORMED);
cv::Mat drawOrbKeypoints(const cv::Mat &src, int maxFeatures = 500);
cv::Mat inpaintImage(const cv::Mat &src, const cv::Mat &mask, double radius = 3.0);
cv::Mat pyramidDown(const cv::Mat &src, int levels = 1);
cv::Mat pyramidUp(const cv::Mat &src, int levels = 1);
cv::Mat affineTransform(const cv::Mat &src, double a00, double a01, double a02,
                        double a10, double a11, double a12,
                        int outputWidth = 0, int outputHeight = 0);

cv::Mat sharpen(const cv::Mat &src);
cv::Mat unsharpMask(const cv::Mat &src, double amount = 1.0, int kernelSize = 5,
                    double sigma = 1.0);

cv::Mat addSaltPepperNoise(const cv::Mat &src, double amount = 0.01, unsigned int seed = 0);
cv::Mat addGaussianNoise(const cv::Mat &src, double mean = 0.0, double stddev = 10.0,
                         unsigned int seed = 0);

cv::Mat convertColor(const cv::Mat &src, int code);
cv::Mat applyColorMapImage(const cv::Mat &src, int colorMap);
cv::Mat normalizeToByte(const cv::Mat &src);
std::map<std::string, std::string> readImageMetadata(const std::string &path);

MorphType parseMorphType(const std::string &name);
int parseColorMap(const std::string &name);
int parseColorConversion(const std::string &name);
int parseTemplateMatchMethod(const std::string &name);
std::vector<std::string> availablePresetFilters();

}  // namespace imgproc

#endif  // IMAGE_PROCESSING_IMAGE_OPS_H
