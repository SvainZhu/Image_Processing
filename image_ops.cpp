#include "image_ops.h"

#include <algorithm>
#include <cmath>
#include <cctype>
#include <map>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

namespace imgproc {
namespace {

void requireImage(const cv::Mat &src, const std::string &name = "image") {
    if (src.empty()) {
        throw std::invalid_argument(name + " is empty");
    }
}

int oddKernel(int kernelSize, const std::string &name) {
    if (kernelSize <= 0) {
        throw std::invalid_argument(name + " must be positive");
    }
    return kernelSize % 2 == 0 ? kernelSize + 1 : kernelSize;
}

std::string normalizeName(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        if (ch == '-' || ch == ' ') {
            return '_';
        }
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

int cvMorphCode(MorphType type) {
    switch (type) {
        case MorphType::Erode:
            return cv::MORPH_ERODE;
        case MorphType::Dilate:
            return cv::MORPH_DILATE;
        case MorphType::Open:
            return cv::MORPH_OPEN;
        case MorphType::Close:
            return cv::MORPH_CLOSE;
        case MorphType::Gradient:
            return cv::MORPH_GRADIENT;
        case MorphType::TopHat:
            return cv::MORPH_TOPHAT;
        case MorphType::BlackHat:
            return cv::MORPH_BLACKHAT;
    }
    throw std::invalid_argument("Unsupported morphology type");
}

std::mt19937 makeRng(unsigned int seed) {
    if (seed == 0) {
        return std::mt19937(std::random_device{}());
    }
    return std::mt19937(seed);
}

cv::Mat ensureBgr(const cv::Mat &src) {
    requireImage(src);
    if (src.channels() == 3) {
        return src.clone();
    }
    cv::Mat dst;
    if (src.channels() == 1) {
        cv::cvtColor(src, dst, cv::COLOR_GRAY2BGR);
        return dst;
    }
    if (src.channels() == 4) {
        cv::cvtColor(src, dst, cv::COLOR_BGRA2BGR);
        return dst;
    }
    throw std::invalid_argument("Expected 1, 3, or 4 channel image");
}

cv::Mat binaryMask(const cv::Mat &src, double thresholdValue, bool useOtsu) {
    const cv::Mat gray = toGray(src);
    cv::Mat mask;
    if (useOtsu) {
        cv::threshold(gray, mask, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    } else {
        cv::threshold(gray, mask, thresholdValue, 255, cv::THRESH_BINARY);
    }
    return mask;
}

cv::Scalar labelColor(int label) {
    const int b = (label * 53) % 255;
    const int g = (label * 97) % 255;
    const int r = (label * 193) % 255;
    return cv::Scalar(b, g, r);
}

double grayMedian(const cv::Mat &gray) {
    int histSize = 256;
    const float range[] = {0.0F, 256.0F};
    const float *histRange = range;
    cv::Mat hist;
    cv::calcHist(&gray, 1, nullptr, cv::Mat(), hist, 1, &histSize, &histRange);

    const int target = (gray.rows * gray.cols + 1) / 2;
    int cumulative = 0;
    for (int i = 0; i < histSize; ++i) {
        cumulative += cvRound(hist.at<float>(i));
        if (cumulative >= target) {
            return i;
        }
    }
    return 0.0;
}

}  // namespace

cv::Mat toGray(const cv::Mat &src) {
    requireImage(src);
    if (src.channels() == 1) {
        return src.clone();
    }
    cv::Mat gray;
    if (src.channels() == 3) {
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
        return gray;
    }
    if (src.channels() == 4) {
        cv::cvtColor(src, gray, cv::COLOR_BGRA2GRAY);
        return gray;
    }
    throw std::invalid_argument("toGray supports 1, 3, or 4 channel images");
}

cv::Mat resizeImage(const cv::Mat &src, int width, int height, bool keepAspect) {
    requireImage(src);
    if (width <= 0 && height <= 0) {
        throw std::invalid_argument("width or height must be positive");
    }

    cv::Size target(width, height);
    if (keepAspect) {
        if (width <= 0) {
            const double scale = static_cast<double>(height) / src.rows;
            target.width = static_cast<int>(std::round(src.cols * scale));
        } else if (height <= 0) {
            const double scale = static_cast<double>(width) / src.cols;
            target.height = static_cast<int>(std::round(src.rows * scale));
        } else {
            const double scale = std::min(static_cast<double>(width) / src.cols,
                                          static_cast<double>(height) / src.rows);
            target.width = static_cast<int>(std::round(src.cols * scale));
            target.height = static_cast<int>(std::round(src.rows * scale));
        }
    }

    cv::Mat dst;
    cv::resize(src, dst, target, 0, 0, cv::INTER_AREA);
    return dst;
}

cv::Mat cropImage(const cv::Mat &src, const cv::Rect &roi) {
    requireImage(src);
    const cv::Rect imageRect(0, 0, src.cols, src.rows);
    const cv::Rect safeRoi = roi & imageRect;
    if (safeRoi.empty()) {
        throw std::invalid_argument("crop region is outside the image");
    }
    return src(safeRoi).clone();
}

cv::Mat rotateImage(const cv::Mat &src, double angle, double scale, const cv::Scalar &border) {
    requireImage(src);
    if (scale <= 0.0) {
        throw std::invalid_argument("scale must be positive");
    }

    const cv::Point2f center((src.cols - 1) / 2.0F, (src.rows - 1) / 2.0F);
    cv::Mat rotation = cv::getRotationMatrix2D(center, angle, scale);
    const double radians = angle * CV_PI / 180.0;
    const double absCos = std::abs(std::cos(radians) * scale);
    const double absSin = std::abs(std::sin(radians) * scale);
    const int boundW = static_cast<int>(src.rows * absSin + src.cols * absCos);
    const int boundH = static_cast<int>(src.rows * absCos + src.cols * absSin);
    rotation.at<double>(0, 2) += boundW / 2.0 - center.x;
    rotation.at<double>(1, 2) += boundH / 2.0 - center.y;

    cv::Mat dst;
    cv::warpAffine(src, dst, rotation, cv::Size(boundW, boundH), cv::INTER_LINEAR,
                   cv::BORDER_CONSTANT, border);
    return dst;
}

cv::Mat flipImage(const cv::Mat &src, int flipCode) {
    requireImage(src);
    if (flipCode != -1 && flipCode != 0 && flipCode != 1) {
        throw std::invalid_argument("flipCode must be -1, 0, or 1");
    }
    cv::Mat dst;
    cv::flip(src, dst, flipCode);
    return dst;
}

cv::Mat histogramImage(const cv::Mat &src, int width, int height) {
    requireImage(src);
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("histogram width and height must be positive");
    }

    const cv::Mat gray = toGray(src);
    const int histSize = 256;
    const float range[] = {0.0F, 256.0F};
    const float *histRange = range;
    cv::Mat hist;
    cv::calcHist(&gray, 1, nullptr, cv::Mat(), hist, 1, &histSize, &histRange);
    cv::normalize(hist, hist, 0, height - 1, cv::NORM_MINMAX);

    cv::Mat canvas(height, width, CV_8UC3, cv::Scalar(255, 255, 255));
    const int binW = std::max(1, width / histSize);
    for (int i = 1; i < histSize; ++i) {
        cv::line(canvas,
                 cv::Point(binW * (i - 1), height - cvRound(hist.at<float>(i - 1))),
                 cv::Point(binW * i, height - cvRound(hist.at<float>(i))),
                 cv::Scalar(40, 40, 40), 2, cv::LINE_AA);
    }
    return canvas;
}

cv::Mat equalizeHistogram(const cv::Mat &src) {
    requireImage(src);
    if (src.channels() == 1) {
        cv::Mat dst;
        cv::equalizeHist(src, dst);
        return dst;
    }

    cv::Mat ycrcb;
    cv::cvtColor(src, ycrcb, cv::COLOR_BGR2YCrCb);
    std::vector<cv::Mat> channels;
    cv::split(ycrcb, channels);
    cv::equalizeHist(channels[0], channels[0]);
    cv::merge(channels, ycrcb);
    cv::Mat dst;
    cv::cvtColor(ycrcb, dst, cv::COLOR_YCrCb2BGR);
    return dst;
}

cv::Mat claheEqualize(const cv::Mat &src, double clipLimit, int tileGridSize) {
    requireImage(src);
    if (clipLimit <= 0.0) {
        throw std::invalid_argument("clipLimit must be positive");
    }
    if (tileGridSize <= 0) {
        throw std::invalid_argument("tileGridSize must be positive");
    }

    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(clipLimit,
                                               cv::Size(tileGridSize, tileGridSize));
    if (src.channels() == 1) {
        cv::Mat dst;
        clahe->apply(src, dst);
        return dst;
    }

    cv::Mat lab;
    cv::cvtColor(ensureBgr(src), lab, cv::COLOR_BGR2Lab);
    std::vector<cv::Mat> channels;
    cv::split(lab, channels);
    clahe->apply(channels[0], channels[0]);
    cv::merge(channels, lab);
    cv::Mat dst;
    cv::cvtColor(lab, dst, cv::COLOR_Lab2BGR);
    return dst;
}

cv::Mat adjustBrightnessContrast(const cv::Mat &src, double alpha, double beta) {
    requireImage(src);
    cv::Mat dst;
    src.convertTo(dst, -1, alpha, beta);
    return dst;
}

cv::Mat gammaCorrection(const cv::Mat &src, double gamma) {
    requireImage(src);
    if (gamma <= 0.0) {
        throw std::invalid_argument("gamma must be positive");
    }
    cv::Mat lut(1, 256, CV_8U);
    const double invGamma = 1.0 / gamma;
    for (int i = 0; i < 256; ++i) {
        lut.at<uchar>(0, i) = cv::saturate_cast<uchar>(
                std::pow(i / 255.0, invGamma) * 255.0);
    }
    cv::Mat dst;
    cv::LUT(src, lut, dst);
    return dst;
}

cv::Mat grayWorldWhiteBalance(const cv::Mat &src) {
    requireImage(src);
    cv::Mat bgr = ensureBgr(src);
    cv::Scalar mean = cv::mean(bgr);
    const double grayMean = (mean[0] + mean[1] + mean[2]) / 3.0;
    std::vector<cv::Mat> channels;
    cv::split(bgr, channels);
    for (int i = 0; i < 3; ++i) {
        const double scale = mean[i] > 0.0 ? grayMean / mean[i] : 1.0;
        channels[i].convertTo(channels[i], channels[i].type(), scale, 0.0);
    }
    cv::Mat dst;
    cv::merge(channels, dst);
    return dst;
}

cv::Mat thresholdBinary(const cv::Mat &src, double thresholdValue, double maxValue) {
    cv::Mat dst;
    cv::threshold(toGray(src), dst, thresholdValue, maxValue, cv::THRESH_BINARY);
    return dst;
}

cv::Mat thresholdOtsu(const cv::Mat &src) {
    cv::Mat dst;
    cv::threshold(toGray(src), dst, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    return dst;
}

cv::Mat adaptiveThresholdImage(const cv::Mat &src, int blockSize, double c, bool gaussian) {
    cv::Mat dst;
    blockSize = oddKernel(blockSize, "blockSize");
    if (blockSize < 3) {
        blockSize = 3;
    }
    cv::adaptiveThreshold(toGray(src), dst, 255,
                          gaussian ? cv::ADAPTIVE_THRESH_GAUSSIAN_C
                                   : cv::ADAPTIVE_THRESH_MEAN_C,
                          cv::THRESH_BINARY, blockSize, c);
    return dst;
}

cv::Mat meanBlurImage(const cv::Mat &src, int kernelSize) {
    requireImage(src);
    cv::Mat dst;
    kernelSize = oddKernel(kernelSize, "kernelSize");
    cv::blur(src, dst, cv::Size(kernelSize, kernelSize));
    return dst;
}

cv::Mat gaussianBlurImage(const cv::Mat &src, int kernelSize, double sigma) {
    requireImage(src);
    cv::Mat dst;
    kernelSize = oddKernel(kernelSize, "kernelSize");
    cv::GaussianBlur(src, dst, cv::Size(kernelSize, kernelSize), sigma);
    return dst;
}

cv::Mat medianBlurImage(const cv::Mat &src, int kernelSize) {
    requireImage(src);
    cv::Mat dst;
    kernelSize = oddKernel(kernelSize, "kernelSize");
    cv::medianBlur(src, dst, kernelSize);
    return dst;
}

cv::Mat bilateralFilterImage(const cv::Mat &src, int diameter, double sigmaColor,
                             double sigmaSpace) {
    requireImage(src);
    if (diameter <= 0) {
        throw std::invalid_argument("diameter must be positive");
    }
    cv::Mat dst;
    cv::bilateralFilter(src, dst, diameter, sigmaColor, sigmaSpace);
    return dst;
}

cv::Mat denoiseNlMeans(const cv::Mat &src, float h, int templateWindowSize,
                       int searchWindowSize) {
    requireImage(src);
    if (h < 0.0F) {
        throw std::invalid_argument("h must be non-negative");
    }
    templateWindowSize = oddKernel(templateWindowSize, "templateWindowSize");
    searchWindowSize = oddKernel(searchWindowSize, "searchWindowSize");

    cv::Mat dst;
    if (src.channels() == 1) {
        cv::fastNlMeansDenoising(src, dst, h, templateWindowSize, searchWindowSize);
    } else {
        cv::fastNlMeansDenoisingColored(ensureBgr(src), dst, h, h, templateWindowSize,
                                        searchWindowSize);
    }
    return dst;
}

cv::Mat morphologyImage(const cv::Mat &src, MorphType type, int kernelSize, int iterations) {
    requireImage(src);
    if (iterations <= 0) {
        throw std::invalid_argument("iterations must be positive");
    }
    kernelSize = oddKernel(kernelSize, "kernelSize");
    const cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT,
                                                       cv::Size(kernelSize, kernelSize));
    cv::Mat dst;
    cv::morphologyEx(src, dst, cvMorphCode(type), element, cv::Point(-1, -1), iterations);
    return dst;
}

cv::Mat cannyEdge(const cv::Mat &src, double lowThreshold, double highThreshold,
                  int apertureSize) {
    cv::Mat edges;
    apertureSize = oddKernel(apertureSize, "apertureSize");
    cv::Canny(toGray(src), edges, lowThreshold, highThreshold, apertureSize);
    return edges;
}

cv::Mat autoCannyEdge(const cv::Mat &src, double sigma, int apertureSize) {
    if (sigma < 0.0 || sigma >= 1.0) {
        throw std::invalid_argument("sigma must be in [0, 1)");
    }
    const cv::Mat gray = toGray(src);
    const double medianValue = grayMedian(gray);
    const double low = std::max(0.0, (1.0 - sigma) * medianValue);
    const double high = std::min(255.0, (1.0 + sigma) * medianValue);
    return cannyEdge(gray, low, high, apertureSize);
}

cv::Mat sobelEdge(const cv::Mat &src, int dx, int dy, int kernelSize) {
    if (dx < 0 || dy < 0 || (dx == 0 && dy == 0)) {
        throw std::invalid_argument("Sobel dx/dy must request at least one derivative");
    }
    kernelSize = oddKernel(kernelSize, "kernelSize");
    cv::Mat grad16, dst;
    cv::Sobel(toGray(src), grad16, CV_16S, dx, dy, kernelSize);
    cv::convertScaleAbs(grad16, dst);
    return dst;
}

cv::Mat laplacianEdge(const cv::Mat &src, int kernelSize) {
    kernelSize = oddKernel(kernelSize, "kernelSize");
    cv::Mat lap16, dst;
    cv::Laplacian(toGray(src), lap16, CV_16S, kernelSize);
    cv::convertScaleAbs(lap16, dst);
    return dst;
}

cv::Mat gradientMagnitude(const cv::Mat &src, int kernelSize, bool useScharr) {
    const cv::Mat gray = toGray(src);
    cv::Mat gx, gy, magnitude, dst;
    if (useScharr) {
        cv::Scharr(gray, gx, CV_32F, 1, 0);
        cv::Scharr(gray, gy, CV_32F, 0, 1);
    } else {
        kernelSize = oddKernel(kernelSize, "kernelSize");
        cv::Sobel(gray, gx, CV_32F, 1, 0, kernelSize);
        cv::Sobel(gray, gy, CV_32F, 0, 1, kernelSize);
    }
    cv::magnitude(gx, gy, magnitude);
    cv::normalize(magnitude, dst, 0, 255, cv::NORM_MINMAX);
    dst.convertTo(dst, CV_8U);
    return dst;
}

cv::Mat contoursImage(const cv::Mat &src, double thresholdValue, bool useOtsu,
                      int thickness) {
    if (thickness <= 0) {
        throw std::invalid_argument("thickness must be positive");
    }
    cv::Mat mask = binaryMask(src, thresholdValue, useOtsu);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::Mat dst = ensureBgr(src);
    cv::drawContours(dst, contours, -1, cv::Scalar(0, 255, 0), thickness, cv::LINE_AA);
    return dst;
}

cv::Mat connectedComponentsImage(const cv::Mat &src, int minArea) {
    if (minArea < 0) {
        throw std::invalid_argument("minArea must be non-negative");
    }
    cv::Mat mask = binaryMask(src, 0.0, true);
    cv::Mat labels, stats, centroids;
    const int count = cv::connectedComponentsWithStats(mask, labels, stats, centroids, 8,
                                                       CV_32S);
    cv::Mat dst(src.size(), CV_8UC3, cv::Scalar(0, 0, 0));
    for (int label = 1; label < count; ++label) {
        const int area = stats.at<int>(label, cv::CC_STAT_AREA);
        if (area < minArea) {
            continue;
        }
        dst.setTo(labelColor(label), labels == label);
    }
    return dst;
}

cv::Mat distanceTransformImage(const cv::Mat &src) {
    cv::Mat mask = binaryMask(src, 0.0, true);
    cv::Mat distance;
    cv::distanceTransform(mask, distance, cv::DIST_L2, 5);
    return normalizeToByte(distance);
}

cv::Mat watershedSegmentation(const cv::Mat &src) {
    cv::Mat bgr = ensureBgr(src);
    cv::Mat gray = toGray(bgr);
    cv::Mat binary;
    cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(binary, binary, cv::MORPH_OPEN, kernel, cv::Point(-1, -1), 2);

    cv::Mat sureBackground;
    cv::dilate(binary, sureBackground, kernel, cv::Point(-1, -1), 3);

    cv::Mat distance;
    cv::distanceTransform(binary, distance, cv::DIST_L2, 5);
    cv::normalize(distance, distance, 0, 1.0, cv::NORM_MINMAX);

    cv::Mat sureForeground;
    cv::threshold(distance, sureForeground, 0.4, 1.0, cv::THRESH_BINARY);
    sureForeground.convertTo(sureForeground, CV_8U, 255);

    cv::Mat markers;
    cv::connectedComponents(sureForeground, markers);
    markers += 1;
    markers.setTo(0, sureBackground - sureForeground);
    cv::watershed(bgr, markers);

    cv::Mat dst(src.size(), CV_8UC3, cv::Scalar(0, 0, 0));
    for (int row = 0; row < markers.rows; ++row) {
        const int *markerRow = markers.ptr<int>(row);
        cv::Vec3b *dstRow = dst.ptr<cv::Vec3b>(row);
        for (int col = 0; col < markers.cols; ++col) {
            const int label = markerRow[col];
            if (label == -1) {
                dstRow[col] = cv::Vec3b(0, 0, 255);
            } else if (label <= 1) {
                dstRow[col] = cv::Vec3b(0, 0, 0);
            } else {
                const cv::Scalar color = labelColor(label);
                dstRow[col] = cv::Vec3b(static_cast<uchar>(color[0]),
                                        static_cast<uchar>(color[1]),
                                        static_cast<uchar>(color[2]));
            }
        }
    }
    return dst;
}

cv::Mat houghLinesImage(const cv::Mat &src, double rho, double theta, int threshold,
                        double minLineLength, double maxLineGap) {
    cv::Mat edges = autoCannyEdge(src);
    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(edges, lines, rho, theta, threshold, minLineLength, maxLineGap);
    cv::Mat dst = ensureBgr(src);
    for (const cv::Vec4i &line : lines) {
        cv::line(dst, cv::Point(line[0], line[1]), cv::Point(line[2], line[3]),
                 cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
    }
    return dst;
}

cv::Mat houghCirclesImage(const cv::Mat &src, double dp, double minDist, double param1,
                          double param2, int minRadius, int maxRadius) {
    cv::Mat gray = toGray(src);
    cv::medianBlur(gray, gray, 5);
    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(gray, circles, cv::HOUGH_GRADIENT, dp, minDist, param1, param2,
                     minRadius, maxRadius);
    cv::Mat dst = ensureBgr(src);
    for (const cv::Vec3f &circle : circles) {
        const cv::Point center(cvRound(circle[0]), cvRound(circle[1]));
        const int radius = cvRound(circle[2]);
        cv::circle(dst, center, radius, cv::Scalar(0, 255, 0), 2, cv::LINE_AA);
        cv::circle(dst, center, 2, cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
    }
    return dst;
}

cv::Mat templateMatchImage(const cv::Mat &src, const cv::Mat &templ, int method) {
    requireImage(src);
    requireImage(templ, "template");
    if (templ.cols > src.cols || templ.rows > src.rows) {
        throw std::invalid_argument("template must not be larger than source image");
    }
    cv::Mat matchSrc = src;
    cv::Mat matchTempl = templ;
    if (src.type() != templ.type()) {
        matchSrc = toGray(src);
        matchTempl = toGray(templ);
    }
    cv::Mat result;
    cv::matchTemplate(matchSrc, matchTempl, result, method);
    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);
    const cv::Point matchLoc = (method == cv::TM_SQDIFF || method == cv::TM_SQDIFF_NORMED)
                                       ? minLoc
                                       : maxLoc;
    cv::Mat dst = ensureBgr(src);
    cv::rectangle(dst, cv::Rect(matchLoc.x, matchLoc.y, templ.cols, templ.rows),
                  cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
    return dst;
}

cv::Mat drawOrbKeypoints(const cv::Mat &src, int maxFeatures) {
    requireImage(src);
    if (maxFeatures <= 0) {
        throw std::invalid_argument("maxFeatures must be positive");
    }
    cv::Ptr<cv::ORB> orb = cv::ORB::create(maxFeatures);
    std::vector<cv::KeyPoint> keypoints;
    cv::Mat descriptors;
    orb->detectAndCompute(toGray(src), cv::noArray(), keypoints, descriptors);
    cv::Mat dst;
    cv::drawKeypoints(src, keypoints, dst, cv::Scalar(0, 255, 0),
                      cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    return dst;
}

cv::Mat inpaintImage(const cv::Mat &src, const cv::Mat &mask, double radius) {
    requireImage(src);
    requireImage(mask, "mask");
    if (radius <= 0.0) {
        throw std::invalid_argument("radius must be positive");
    }
    cv::Mat binary = toGray(mask);
    cv::threshold(binary, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    cv::Mat dst;
    cv::inpaint(src, binary, dst, radius, cv::INPAINT_TELEA);
    return dst;
}

cv::Mat pyramidDown(const cv::Mat &src, int levels) {
    requireImage(src);
    if (levels <= 0) {
        throw std::invalid_argument("levels must be positive");
    }
    cv::Mat dst = src.clone();
    for (int i = 0; i < levels; ++i) {
        cv::pyrDown(dst, dst);
    }
    return dst;
}

cv::Mat pyramidUp(const cv::Mat &src, int levels) {
    requireImage(src);
    if (levels <= 0) {
        throw std::invalid_argument("levels must be positive");
    }
    cv::Mat dst = src.clone();
    for (int i = 0; i < levels; ++i) {
        cv::pyrUp(dst, dst);
    }
    return dst;
}

cv::Mat affineTransform(const cv::Mat &src, double a00, double a01, double a02,
                        double a10, double a11, double a12,
                        int outputWidth, int outputHeight) {
    requireImage(src);
    if (outputWidth <= 0) {
        outputWidth = src.cols;
    }
    if (outputHeight <= 0) {
        outputHeight = src.rows;
    }
    cv::Mat matrix = (cv::Mat_<double>(2, 3) << a00, a01, a02, a10, a11, a12);
    cv::Mat dst;
    cv::warpAffine(src, dst, matrix, cv::Size(outputWidth, outputHeight),
                   cv::INTER_LINEAR, cv::BORDER_REFLECT101);
    return dst;
}

cv::Mat sharpen(const cv::Mat &src) {
    requireImage(src);
    const cv::Mat kernel = (cv::Mat_<float>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
    cv::Mat dst;
    cv::filter2D(src, dst, src.depth(), kernel);
    return dst;
}

cv::Mat unsharpMask(const cv::Mat &src, double amount, int kernelSize, double sigma) {
    requireImage(src);
    if (amount < 0.0) {
        throw std::invalid_argument("amount must be non-negative");
    }
    cv::Mat blurred, dst;
    kernelSize = oddKernel(kernelSize, "kernelSize");
    cv::GaussianBlur(src, blurred, cv::Size(kernelSize, kernelSize), sigma);
    cv::addWeighted(src, 1.0 + amount, blurred, -amount, 0, dst);
    return dst;
}

cv::Mat addSaltPepperNoise(const cv::Mat &src, double amount, unsigned int seed) {
    requireImage(src);
    if (amount < 0.0 || amount > 1.0) {
        throw std::invalid_argument("salt-pepper amount must be in [0, 1]");
    }
    cv::Mat dst = src.clone();
    const int total = src.rows * src.cols;
    const int noisyPixels = static_cast<int>(std::round(total * amount));
    std::mt19937 rng = makeRng(seed);
    std::uniform_int_distribution<int> rowDist(0, src.rows - 1);
    std::uniform_int_distribution<int> colDist(0, src.cols - 1);
    std::bernoulli_distribution saltOrPepper(0.5);

    for (int k = 0; k < noisyPixels; ++k) {
        const int row = rowDist(rng);
        const int col = colDist(rng);
        const uchar value = saltOrPepper(rng) ? 255 : 0;
        if (dst.channels() == 1) {
            dst.at<uchar>(row, col) = value;
        } else {
            for (int channel = 0; channel < dst.channels(); ++channel) {
                dst.ptr<uchar>(row)[col * dst.channels() + channel] = value;
            }
        }
    }
    return dst;
}

cv::Mat addGaussianNoise(const cv::Mat &src, double mean, double stddev, unsigned int seed) {
    requireImage(src);
    if (stddev < 0.0) {
        throw std::invalid_argument("stddev must be non-negative");
    }
    cv::Mat src32;
    src.convertTo(src32, CV_MAKETYPE(CV_32F, src.channels()));
    cv::Mat noise(src.size(), src32.type());
    if (seed != 0) {
        cv::theRNG().state = seed;
    }
    cv::randn(noise, cv::Scalar::all(mean), cv::Scalar::all(stddev));
    cv::Mat noisy32 = src32 + noise;
    cv::Mat dst;
    noisy32.convertTo(dst, src.type());
    return dst;
}

cv::Mat convertColor(const cv::Mat &src, int code) {
    requireImage(src);
    cv::Mat dst;
    cv::cvtColor(src, dst, code);
    return dst;
}

cv::Mat applyColorMapImage(const cv::Mat &src, int colorMap) {
    cv::Mat dst;
    cv::applyColorMap(normalizeToByte(toGray(src)), dst, colorMap);
    return dst;
}

cv::Mat normalizeToByte(const cv::Mat &src) {
    requireImage(src);
    if (src.depth() == CV_8U) {
        return src.clone();
    }
    cv::Mat normalized;
    cv::normalize(src, normalized, 0, 255, cv::NORM_MINMAX);
    normalized.convertTo(normalized, CV_8U);
    return normalized;
}

MorphType parseMorphType(const std::string &name) {
    const std::string key = normalizeName(name);
    if (key == "erode") {
        return MorphType::Erode;
    }
    if (key == "dilate") {
        return MorphType::Dilate;
    }
    if (key == "open") {
        return MorphType::Open;
    }
    if (key == "close") {
        return MorphType::Close;
    }
    if (key == "gradient") {
        return MorphType::Gradient;
    }
    if (key == "tophat" || key == "top_hat") {
        return MorphType::TopHat;
    }
    if (key == "blackhat" || key == "black_hat") {
        return MorphType::BlackHat;
    }
    throw std::invalid_argument("Unknown morphology type: " + name);
}

int parseColorMap(const std::string &name) {
    static const std::map<std::string, int> maps = {
            {"autumn", cv::COLORMAP_AUTUMN}, {"bone", cv::COLORMAP_BONE},
            {"jet", cv::COLORMAP_JET},       {"winter", cv::COLORMAP_WINTER},
            {"rainbow", cv::COLORMAP_RAINBOW}, {"ocean", cv::COLORMAP_OCEAN},
            {"summer", cv::COLORMAP_SUMMER}, {"spring", cv::COLORMAP_SPRING},
            {"cool", cv::COLORMAP_COOL},     {"hsv", cv::COLORMAP_HSV},
            {"pink", cv::COLORMAP_PINK},     {"hot", cv::COLORMAP_HOT},
            {"parula", cv::COLORMAP_PARULA}, {"magma", cv::COLORMAP_MAGMA},
            {"inferno", cv::COLORMAP_INFERNO}, {"plasma", cv::COLORMAP_PLASMA},
            {"viridis", cv::COLORMAP_VIRIDIS}, {"cividis", cv::COLORMAP_CIVIDIS},
            {"twilight", cv::COLORMAP_TWILIGHT},
            {"twilight_shifted", cv::COLORMAP_TWILIGHT_SHIFTED},
            {"turbo", cv::COLORMAP_TURBO},
            {"deepgreen", cv::COLORMAP_DEEPGREEN},
    };
    const auto it = maps.find(normalizeName(name));
    if (it == maps.end()) {
        throw std::invalid_argument("Unknown color map: " + name);
    }
    return it->second;
}

int parseColorConversion(const std::string &name) {
    static const std::map<std::string, int> codes = {
            {"bgr2rgb", cv::COLOR_BGR2RGB},   {"rgb2bgr", cv::COLOR_RGB2BGR},
            {"bgr2gray", cv::COLOR_BGR2GRAY}, {"gray2bgr", cv::COLOR_GRAY2BGR},
            {"bgr2hsv", cv::COLOR_BGR2HSV},   {"hsv2bgr", cv::COLOR_HSV2BGR},
            {"bgr2lab", cv::COLOR_BGR2Lab},   {"lab2bgr", cv::COLOR_Lab2BGR},
            {"bgr2ycrcb", cv::COLOR_BGR2YCrCb},
            {"ycrcb2bgr", cv::COLOR_YCrCb2BGR},
    };
    const auto it = codes.find(normalizeName(name));
    if (it == codes.end()) {
        throw std::invalid_argument("Unknown color conversion: " + name);
    }
    return it->second;
}

int parseTemplateMatchMethod(const std::string &name) {
    static const std::map<std::string, int> methods = {
            {"sqdiff", cv::TM_SQDIFF},
            {"sqdiff_normed", cv::TM_SQDIFF_NORMED},
            {"ccorr", cv::TM_CCORR},
            {"ccorr_normed", cv::TM_CCORR_NORMED},
            {"ccoeff", cv::TM_CCOEFF},
            {"ccoeff_normed", cv::TM_CCOEFF_NORMED},
    };
    const auto it = methods.find(normalizeName(name));
    if (it == methods.end()) {
        throw std::invalid_argument("Unknown template match method: " + name);
    }
    return it->second;
}

}  // namespace imgproc
