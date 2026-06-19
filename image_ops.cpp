#include "image_ops.h"

#include <algorithm>
#include <cmath>
#include <cctype>
#include <map>
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

}  // namespace imgproc
