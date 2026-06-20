#include "image_ops.h"

#include <algorithm>
#include <cmath>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <limits>
#include <map>
#include <numeric>
#include <random>
#include <sstream>
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

double clampDouble(double value, double low, double high) {
    return std::max(low, std::min(high, value));
}

int clampByte(double value) {
    return static_cast<int>(clampDouble(std::round(value), 0.0, 255.0));
}

cv::Mat blendImages(const cv::Mat &base, const cv::Mat &adjusted, double intensity) {
    intensity = clampDouble(intensity, 0.0, 1.0);
    if (intensity >= 1.0) {
        return adjusted.clone();
    }
    if (intensity <= 0.0) {
        return base.clone();
    }
    cv::Mat dst;
    cv::addWeighted(base, 1.0 - intensity, adjusted, intensity, 0.0, dst);
    return dst;
}

double bytePercentile(const cv::Mat &gray, double percentile) {
    int histSize = 256;
    const float range[] = {0.0F, 256.0F};
    const float *histRange = range;
    cv::Mat hist;
    cv::calcHist(&gray, 1, nullptr, cv::Mat(), hist, 1, &histSize, &histRange);

    const int total = gray.rows * gray.cols;
    const int target = std::max(1, static_cast<int>(std::ceil(total * percentile)));
    int cumulative = 0;
    for (int i = 0; i < histSize; ++i) {
        cumulative += cvRound(hist.at<float>(i));
        if (cumulative >= target) {
            return i;
        }
    }
    return 255.0;
}

cv::Mat autoLevelsLab(const cv::Mat &src, double lowPercentile = 0.005,
                      double highPercentile = 0.995) {
    cv::Mat lab;
    cv::cvtColor(ensureBgr(src), lab, cv::COLOR_BGR2Lab);
    std::vector<cv::Mat> channels;
    cv::split(lab, channels);

    const double low = bytePercentile(channels[0], lowPercentile);
    const double high = bytePercentile(channels[0], highPercentile);
    if (high - low < 8.0) {
        return ensureBgr(src);
    }

    cv::Mat lut(1, 256, CV_8U);
    const double scale = 255.0 / (high - low);
    for (int i = 0; i < 256; ++i) {
        lut.at<uchar>(0, i) = cv::saturate_cast<uchar>((i - low) * scale);
    }
    cv::LUT(channels[0], lut, channels[0]);
    cv::merge(channels, lab);

    cv::Mat dst;
    cv::cvtColor(lab, dst, cv::COLOR_Lab2BGR);
    return dst;
}

cv::Mat buildCurveLut(std::vector<cv::Point2f> points) {
    if (points.size() < 2) {
        throw std::invalid_argument("tone curve needs at least two points");
    }

    for (cv::Point2f &point : points) {
        if (!std::isfinite(point.x) || !std::isfinite(point.y)) {
            throw std::invalid_argument("tone curve points must be finite");
        }
        point.x = static_cast<float>(clampDouble(point.x, 0.0, 255.0));
        point.y = static_cast<float>(clampDouble(point.y, 0.0, 255.0));
    }
    std::sort(points.begin(), points.end(), [](const cv::Point2f &lhs,
                                               const cv::Point2f &rhs) {
        return lhs.x < rhs.x;
    });

    std::vector<cv::Point2f> uniquePoints;
    uniquePoints.reserve(points.size() + 2);
    for (const cv::Point2f &point : points) {
        if (!uniquePoints.empty() && std::abs(uniquePoints.back().x - point.x) < 0.001F) {
            uniquePoints.back().y = point.y;
        } else {
            uniquePoints.push_back(point);
        }
    }
    if (uniquePoints.size() < 2) {
        throw std::invalid_argument("tone curve x values must contain at least two positions");
    }
    if (uniquePoints.front().x > 0.0F) {
        uniquePoints.insert(uniquePoints.begin(), cv::Point2f(0.0F, uniquePoints.front().y));
    }
    if (uniquePoints.back().x < 255.0F) {
        uniquePoints.push_back(cv::Point2f(255.0F, uniquePoints.back().y));
    }

    cv::Mat lut(1, 256, CV_8U);
    std::size_t segment = 0;
    for (int x = 0; x < 256; ++x) {
        while (segment + 1 < uniquePoints.size() &&
               x > uniquePoints[segment + 1].x) {
            ++segment;
        }
        const cv::Point2f &p0 = uniquePoints[segment];
        const cv::Point2f &p1 = uniquePoints[std::min(segment + 1, uniquePoints.size() - 1)];
        const double span = std::max(1.0F, p1.x - p0.x);
        const double t = clampDouble((x - p0.x) / span, 0.0, 1.0);
        lut.at<uchar>(0, x) = static_cast<uchar>(clampByte(p0.y + (p1.y - p0.y) * t));
    }
    return lut;
}

std::vector<unsigned char> readFileBytes(const std::string &path) {
    std::ifstream input(path.c_str(), std::ios::binary);
    if (!input) {
        throw std::runtime_error("Cannot open metadata file: " + path);
    }
    input.seekg(0, std::ios::end);
    const std::streamoff size = input.tellg();
    input.seekg(0, std::ios::beg);
    if (size <= 0) {
        return std::vector<unsigned char>();
    }
    std::vector<unsigned char> bytes(static_cast<std::size_t>(size));
    input.read(reinterpret_cast<char *>(bytes.data()), size);
    return bytes;
}

bool hasBytes(const std::vector<unsigned char> &data, std::size_t pos, std::size_t count) {
    return pos <= data.size() && count <= data.size() - pos;
}

uint16_t readBe16(const std::vector<unsigned char> &data, std::size_t pos) {
    if (!hasBytes(data, pos, 2)) {
        return 0;
    }
    return static_cast<uint16_t>((data[pos] << 8) | data[pos + 1]);
}

class ExifReader {
public:
    ExifReader(const std::vector<unsigned char> &bytes, std::size_t tiffStart, bool littleEndian)
            : data(bytes), tiff(tiffStart), little(littleEndian) {}

    void parseIfd(uint32_t offset, bool exifIfd, std::map<std::string, std::string> &metadata) const {
        const std::size_t ifd = tiff + offset;
        if (!hasBytes(data, ifd, 2)) {
            return;
        }
        const uint16_t count = read16(ifd);
        for (uint16_t i = 0; i < count; ++i) {
            const std::size_t entry = ifd + 2 + static_cast<std::size_t>(i) * 12;
            if (!hasBytes(data, entry, 12)) {
                break;
            }

            const uint16_t tag = read16(entry);
            const uint16_t type = read16(entry + 2);
            const uint32_t valueCount = read32(entry + 4);
            const std::size_t value = valuePosition(entry, type, valueCount);
            if (value == std::numeric_limits<std::size_t>::max()) {
                continue;
            }

            if (!exifIfd && tag == 0x8769) {
                parseIfd(readFirstUnsigned(type, value), true, metadata);
                continue;
            }
            addKnownTag(tag, type, valueCount, value, exifIfd, metadata);
        }
    }

private:
    const std::vector<unsigned char> &data;
    std::size_t tiff;
    bool little;

    uint16_t read16(std::size_t pos) const {
        if (!hasBytes(data, pos, 2)) {
            return 0;
        }
        if (little) {
            return static_cast<uint16_t>(data[pos] | (data[pos + 1] << 8));
        }
        return static_cast<uint16_t>((data[pos] << 8) | data[pos + 1]);
    }

    int32_t readS32(std::size_t pos) const {
        return static_cast<int32_t>(read32(pos));
    }

    uint32_t read32(std::size_t pos) const {
        if (!hasBytes(data, pos, 4)) {
            return 0;
        }
        if (little) {
            return static_cast<uint32_t>(data[pos]) |
                   (static_cast<uint32_t>(data[pos + 1]) << 8) |
                   (static_cast<uint32_t>(data[pos + 2]) << 16) |
                   (static_cast<uint32_t>(data[pos + 3]) << 24);
        }
        return (static_cast<uint32_t>(data[pos]) << 24) |
               (static_cast<uint32_t>(data[pos + 1]) << 16) |
               (static_cast<uint32_t>(data[pos + 2]) << 8) |
               static_cast<uint32_t>(data[pos + 3]);
    }

    std::size_t typeSize(uint16_t type) const {
        switch (type) {
            case 1:
            case 2:
            case 6:
            case 7:
                return 1;
            case 3:
            case 8:
                return 2;
            case 4:
            case 9:
            case 11:
                return 4;
            case 5:
            case 10:
            case 12:
                return 8;
            default:
                return 0;
        }
    }

    std::size_t valuePosition(std::size_t entry, uint16_t type, uint32_t count) const {
        const std::size_t itemSize = typeSize(type);
        if (itemSize == 0 || count == 0 ||
            count > std::numeric_limits<std::size_t>::max() / itemSize) {
            return std::numeric_limits<std::size_t>::max();
        }
        const std::size_t totalSize = itemSize * count;
        if (totalSize <= 4) {
            return entry + 8;
        }
        const uint32_t offset = read32(entry + 8);
        const std::size_t pos = tiff + offset;
        if (!hasBytes(data, pos, totalSize)) {
            return std::numeric_limits<std::size_t>::max();
        }
        return pos;
    }

    std::string readAscii(std::size_t pos, uint32_t count) const {
        if (!hasBytes(data, pos, count)) {
            return std::string();
        }
        std::string value(reinterpret_cast<const char *>(&data[pos]),
                          reinterpret_cast<const char *>(&data[pos + count]));
        while (!value.empty() && (value.back() == '\0' || std::isspace(static_cast<unsigned char>(value.back())))) {
            value.pop_back();
        }
        while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
            value.erase(value.begin());
        }
        return value;
    }

    uint32_t readFirstUnsigned(uint16_t type, std::size_t pos) const {
        if (type == 3) {
            return read16(pos);
        }
        if (type == 4) {
            return read32(pos);
        }
        if (type == 1 && hasBytes(data, pos, 1)) {
            return data[pos];
        }
        return 0;
    }

    double readRational(uint16_t type, std::size_t pos) const {
        if (type == 5) {
            const uint32_t numerator = read32(pos);
            const uint32_t denominator = read32(pos + 4);
            return denominator == 0 ? 0.0 : static_cast<double>(numerator) / denominator;
        }
        if (type == 10) {
            const int32_t numerator = readS32(pos);
            const int32_t denominator = readS32(pos + 4);
            return denominator == 0 ? 0.0 : static_cast<double>(numerator) / denominator;
        }
        return static_cast<double>(readFirstUnsigned(type, pos));
    }

    std::string formatDouble(double value, int precision = 2) const {
        std::ostringstream out;
        out << std::fixed << std::setprecision(precision) << value;
        std::string text = out.str();
        while (text.size() > 1 && text.back() == '0') {
            text.pop_back();
        }
        if (!text.empty() && text.back() == '.') {
            text.pop_back();
        }
        return text;
    }

    std::string formatExposure(double seconds) const {
        if (seconds > 0.0 && seconds < 1.0) {
            const int denominator = static_cast<int>(std::round(1.0 / seconds));
            if (denominator > 0) {
                return "1/" + std::to_string(denominator);
            }
        }
        return formatDouble(seconds, 4);
    }

    void setIfPresent(std::map<std::string, std::string> &metadata,
                      const std::string &key, const std::string &value) const {
        if (!value.empty()) {
            metadata[key] = value;
        }
    }

    void addKnownTag(uint16_t tag, uint16_t type, uint32_t count, std::size_t value,
                     bool exifIfd, std::map<std::string, std::string> &metadata) const {
        if (!exifIfd) {
            switch (tag) {
                case 0x010F:
                    setIfPresent(metadata, "make", readAscii(value, count));
                    break;
                case 0x0110:
                    setIfPresent(metadata, "model", readAscii(value, count));
                    break;
                case 0x0112:
                    metadata["orientation"] = std::to_string(readFirstUnsigned(type, value));
                    break;
                case 0x0131:
                    setIfPresent(metadata, "software", readAscii(value, count));
                    break;
                case 0x0132:
                    setIfPresent(metadata, "datetime", readAscii(value, count));
                    break;
                default:
                    break;
            }
            return;
        }

        switch (tag) {
            case 0x829A:
                metadata["exposure_time_s"] = formatExposure(readRational(type, value));
                break;
            case 0x829D:
                metadata["aperture"] = "f/" + formatDouble(readRational(type, value), 2);
                break;
            case 0x8827:
                metadata["iso"] = std::to_string(readFirstUnsigned(type, value));
                break;
            case 0x9003:
                setIfPresent(metadata, "datetime_original", readAscii(value, count));
                break;
            case 0x9204:
                metadata["exposure_bias_ev"] = formatDouble(readRational(type, value), 2);
                break;
            case 0x9209:
                metadata["flash"] = std::to_string(readFirstUnsigned(type, value));
                break;
            case 0x920A:
                metadata["focal_length_mm"] = formatDouble(readRational(type, value), 2);
                break;
            case 0xA002:
                metadata["pixel_width"] = std::to_string(readFirstUnsigned(type, value));
                break;
            case 0xA003:
                metadata["pixel_height"] = std::to_string(readFirstUnsigned(type, value));
                break;
            case 0xA403:
                metadata["white_balance"] = readFirstUnsigned(type, value) == 0 ? "auto" : "manual";
                break;
            case 0xA405:
                metadata["focal_length_35mm"] = std::to_string(readFirstUnsigned(type, value));
                break;
            case 0xA434:
                setIfPresent(metadata, "lens_model", readAscii(value, count));
                break;
            default:
                break;
        }
    }
};

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

cv::Mat adjustSaturation(const cv::Mat &src, double factor) {
    requireImage(src);
    if (factor < 0.0) {
        throw std::invalid_argument("saturation factor must be non-negative");
    }
    if (src.channels() == 1) {
        return src.clone();
    }

    cv::Mat hsv;
    cv::cvtColor(ensureBgr(src), hsv, cv::COLOR_BGR2HSV);
    std::vector<cv::Mat> channels;
    cv::split(hsv, channels);
    channels[1].convertTo(channels[1], channels[1].type(), factor, 0.0);
    cv::merge(channels, hsv);

    cv::Mat dst;
    cv::cvtColor(hsv, dst, cv::COLOR_HSV2BGR);
    return dst;
}

cv::Mat adjustVibrance(const cv::Mat &src, double amount) {
    requireImage(src);
    if (amount < -1.0 || amount > 1.0) {
        throw std::invalid_argument("vibrance amount must be in [-1, 1]");
    }
    if (src.channels() == 1 || amount == 0.0) {
        return src.clone();
    }

    cv::Mat hsv;
    cv::cvtColor(ensureBgr(src), hsv, cv::COLOR_BGR2HSV);
    std::vector<cv::Mat> channels;
    cv::split(hsv, channels);

    cv::Mat saturation32;
    channels[1].convertTo(saturation32, CV_32F);
    if (amount > 0.0) {
        cv::Mat saturationNorm = saturation32 / 255.0;
        cv::Mat lift = 255.0 - saturation32;
        cv::Mat protection = 1.0 - saturationNorm;
        saturation32 = saturation32 + lift.mul(protection) * amount;
    } else {
        saturation32 = saturation32 * (1.0 + amount);
    }
    saturation32.convertTo(channels[1], CV_8U);
    cv::merge(channels, hsv);

    cv::Mat dst;
    cv::cvtColor(hsv, dst, cv::COLOR_HSV2BGR);
    return dst;
}

cv::Mat adjustTemperatureTint(const cv::Mat &src, double temperature, double tint) {
    requireImage(src);
    if (temperature < -1.0 || temperature > 1.0 || tint < -1.0 || tint > 1.0) {
        throw std::invalid_argument("temperature and tint must be in [-1, 1]");
    }
    if (src.channels() == 1) {
        return src.clone();
    }

    cv::Mat lab;
    cv::cvtColor(ensureBgr(src), lab, cv::COLOR_BGR2Lab);
    std::vector<cv::Mat> channels;
    cv::split(lab, channels);
    channels[1].convertTo(channels[1], channels[1].type(), 1.0, tint * 32.0);
    channels[2].convertTo(channels[2], channels[2].type(), 1.0, temperature * 32.0);
    cv::merge(channels, lab);

    cv::Mat dst;
    cv::cvtColor(lab, dst, cv::COLOR_Lab2BGR);
    return dst;
}

cv::Mat applyToneCurve(const cv::Mat &src, const std::vector<cv::Point2f> &points,
                       const std::string &channel) {
    requireImage(src);
    const cv::Mat lut = buildCurveLut(points);
    const std::string key = normalizeName(channel);
    if (key == "all" || key == "rgb" || src.channels() == 1) {
        cv::Mat dst;
        cv::LUT(src, lut, dst);
        return dst;
    }

    cv::Mat bgr = ensureBgr(src);
    std::vector<cv::Mat> channels;
    cv::split(bgr, channels);
    int index = -1;
    if (key == "b" || key == "blue") {
        index = 0;
    } else if (key == "g" || key == "green") {
        index = 1;
    } else if (key == "r" || key == "red") {
        index = 2;
    } else {
        throw std::invalid_argument("curve channel must be all, r, g, or b");
    }
    cv::LUT(channels[index], lut, channels[index]);
    cv::Mat dst;
    cv::merge(channels, dst);
    return dst;
}

cv::Mat applyPresetFilter(const cv::Mat &src, const std::string &preset,
                          double intensity) {
    requireImage(src);
    if (intensity < 0.0 || intensity > 1.0) {
        throw std::invalid_argument("filter intensity must be in [0, 1]");
    }

    const std::string key = normalizeName(preset);
    const cv::Mat base = ensureBgr(src);
    cv::Mat work = base.clone();

    if (key == "vivid") {
        work = applyToneCurve(work, {cv::Point2f(0, 0), cv::Point2f(48, 42),
                                    cv::Point2f(128, 140), cv::Point2f(210, 232),
                                    cv::Point2f(255, 255)});
        work = adjustVibrance(work, 0.38);
        work = unsharpMask(work, 0.25, 3, 0.7);
    } else if (key == "landscape") {
        work = claheEqualize(work, 2.2, 8);
        work = adjustVibrance(work, 0.32);
        work = adjustTemperatureTint(work, -0.06, -0.04);
        work = unsharpMask(work, 0.35, 3, 0.8);
    } else if (key == "portrait") {
        work = adjustTemperatureTint(work, 0.08, 0.04);
        work = applyToneCurve(work, {cv::Point2f(0, 6), cv::Point2f(72, 78),
                                    cv::Point2f(160, 166), cv::Point2f(255, 250)});
        work = adjustSaturation(work, 1.04);
    } else if (key == "food") {
        work = adjustTemperatureTint(work, 0.12, 0.02);
        work = adjustVibrance(work, 0.42);
        work = applyToneCurve(work, {cv::Point2f(0, 0), cv::Point2f(90, 88),
                                    cv::Point2f(170, 188), cv::Point2f(255, 255)});
    } else if (key == "night") {
        work = denoiseNlMeans(work, 6.0F, 7, 21);
        work = claheEqualize(work, 1.8, 8);
        work = applyToneCurve(work, {cv::Point2f(0, 2), cv::Point2f(56, 66),
                                    cv::Point2f(160, 176), cv::Point2f(255, 255)});
    } else if (key == "document") {
        cv::Mat gray = toGray(work);
        cv::Mat enhanced;
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.5, cv::Size(8, 8));
        clahe->apply(gray, enhanced);
        cv::Mat binary;
        cv::adaptiveThreshold(enhanced, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                              cv::THRESH_BINARY, 31, 7);
        cv::cvtColor(binary, work, cv::COLOR_GRAY2BGR);
    } else if (key == "cinematic") {
        work = adjustTemperatureTint(work, -0.12, 0.06);
        work = adjustSaturation(work, 0.82);
        work = applyToneCurve(work, {cv::Point2f(0, 10), cv::Point2f(72, 62),
                                    cv::Point2f(180, 196), cv::Point2f(255, 244)});
    } else if (key == "warm") {
        work = adjustTemperatureTint(work, 0.22, 0.03);
        work = adjustVibrance(work, 0.12);
    } else if (key == "cool") {
        work = adjustTemperatureTint(work, -0.22, -0.02);
        work = adjustVibrance(work, 0.08);
    } else if (key == "mono" || key == "black_white" || key == "bw") {
        cv::cvtColor(toGray(work), work, cv::COLOR_GRAY2BGR);
        work = applyToneCurve(work, {cv::Point2f(0, 0), cv::Point2f(70, 58),
                                    cv::Point2f(180, 202), cv::Point2f(255, 255)});
    } else if (key == "fade") {
        work = adjustSaturation(work, 0.82);
        work = applyToneCurve(work, {cv::Point2f(0, 22), cv::Point2f(92, 96),
                                    cv::Point2f(190, 198), cv::Point2f(255, 244)});
    } else if (key == "vintage") {
        work = adjustTemperatureTint(work, 0.18, 0.08);
        work = adjustSaturation(work, 0.86);
        work = applyToneCurve(work, {cv::Point2f(0, 18), cv::Point2f(80, 74),
                                    cv::Point2f(170, 185), cv::Point2f(255, 240)});
        work = applyToneCurve(work, {cv::Point2f(0, 10), cv::Point2f(128, 126),
                                    cv::Point2f(255, 242)}, "b");
    } else {
        throw std::invalid_argument("Unknown preset filter: " + preset);
    }

    return blendImages(base, work, intensity);
}

cv::Mat autoEnhance(const cv::Mat &src, double strength) {
    requireImage(src);
    if (strength < 0.0 || strength > 1.0) {
        throw std::invalid_argument("auto enhance strength must be in [0, 1]");
    }

    const cv::Mat base = ensureBgr(src);
    cv::Mat work = grayWorldWhiteBalance(base);
    work = autoLevelsLab(work);
    work = claheEqualize(work, 1.7, 8);

    cv::Mat lab;
    cv::cvtColor(work, lab, cv::COLOR_BGR2Lab);
    std::vector<cv::Mat> labChannels;
    cv::split(lab, labChannels);
    cv::Scalar meanL, stdL;
    cv::meanStdDev(labChannels[0], meanL, stdL);

    const double contrastGain = clampDouble(1.0 + (48.0 - stdL[0]) / 420.0, 0.94, 1.14);
    const double lightBias = clampDouble((128.0 - meanL[0]) / 18.0, -8.0, 8.0);
    work = adjustBrightnessContrast(work, contrastGain, lightBias);

    cv::Mat hsv;
    cv::cvtColor(work, hsv, cv::COLOR_BGR2HSV);
    std::vector<cv::Mat> hsvChannels;
    cv::split(hsv, hsvChannels);
    const double meanSaturation = cv::mean(hsvChannels[1])[0];
    const double vibrance = clampDouble((110.0 - meanSaturation) / 260.0, 0.08, 0.28);
    work = adjustVibrance(work, vibrance);
    work = unsharpMask(work, 0.22, 3, 0.8);

    return blendImages(base, work, strength);
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

std::map<std::string, std::string> readImageMetadata(const std::string &path) {
    const std::vector<unsigned char> data = readFileBytes(path);
    std::map<std::string, std::string> metadata;
    metadata["file_size_bytes"] = std::to_string(data.size());
    if (data.empty()) {
        return metadata;
    }

    if (hasBytes(data, 0, 8) && data[0] == 0x89 && data[1] == 'P' &&
        data[2] == 'N' && data[3] == 'G') {
        metadata["format"] = "PNG";
        if (hasBytes(data, 16, 8)) {
            const uint32_t width = (static_cast<uint32_t>(data[16]) << 24) |
                                   (static_cast<uint32_t>(data[17]) << 16) |
                                   (static_cast<uint32_t>(data[18]) << 8) |
                                   static_cast<uint32_t>(data[19]);
            const uint32_t height = (static_cast<uint32_t>(data[20]) << 24) |
                                    (static_cast<uint32_t>(data[21]) << 16) |
                                    (static_cast<uint32_t>(data[22]) << 8) |
                                    static_cast<uint32_t>(data[23]);
            metadata["pixel_width"] = std::to_string(width);
            metadata["pixel_height"] = std::to_string(height);
        }
        return metadata;
    }

    if (!hasBytes(data, 0, 2) || data[0] != 0xFF || data[1] != 0xD8) {
        metadata["format"] = "unknown";
        return metadata;
    }

    metadata["format"] = "JPEG";
    std::size_t pos = 2;
    while (hasBytes(data, pos, 4)) {
        while (pos < data.size() && data[pos] == 0xFF) {
            ++pos;
        }
        if (pos >= data.size()) {
            break;
        }

        const unsigned char marker = data[pos++];
        if (marker == 0xD9 || marker == 0xDA) {
            break;
        }
        if (!hasBytes(data, pos, 2)) {
            break;
        }
        const uint16_t segmentLength = readBe16(data, pos);
        if (segmentLength < 2 || !hasBytes(data, pos + 2, segmentLength - 2)) {
            break;
        }

        const std::size_t segmentStart = pos + 2;
        const std::size_t segmentEnd = segmentStart + segmentLength - 2;
        const bool startOfFrame = (marker >= 0xC0 && marker <= 0xC3) ||
                                  (marker >= 0xC5 && marker <= 0xC7) ||
                                  (marker >= 0xC9 && marker <= 0xCB) ||
                                  (marker >= 0xCD && marker <= 0xCF);
        if (startOfFrame && hasBytes(data, segmentStart, 5)) {
            metadata["pixel_height"] = std::to_string(readBe16(data, segmentStart + 1));
            metadata["pixel_width"] = std::to_string(readBe16(data, segmentStart + 3));
        }

        if (marker == 0xE1 && hasBytes(data, segmentStart, 14) &&
            data[segmentStart] == 'E' && data[segmentStart + 1] == 'x' &&
            data[segmentStart + 2] == 'i' && data[segmentStart + 3] == 'f' &&
            data[segmentStart + 4] == 0 && data[segmentStart + 5] == 0) {
            const std::size_t tiff = segmentStart + 6;
            if (hasBytes(data, tiff, 8)) {
                const bool little = data[tiff] == 'I' && data[tiff + 1] == 'I';
                const bool big = data[tiff] == 'M' && data[tiff + 1] == 'M';
                if (little || big) {
                    const ExifReader reader(data, tiff, little);
                    const uint16_t magic = little
                                                   ? static_cast<uint16_t>(data[tiff + 2] |
                                                                           (data[tiff + 3] << 8))
                                                   : static_cast<uint16_t>((data[tiff + 2] << 8) |
                                                                           data[tiff + 3]);
                    const uint32_t ifd0 = little
                                                  ? static_cast<uint32_t>(data[tiff + 4]) |
                                                            (static_cast<uint32_t>(data[tiff + 5]) << 8) |
                                                            (static_cast<uint32_t>(data[tiff + 6]) << 16) |
                                                            (static_cast<uint32_t>(data[tiff + 7]) << 24)
                                                  : (static_cast<uint32_t>(data[tiff + 4]) << 24) |
                                                            (static_cast<uint32_t>(data[tiff + 5]) << 16) |
                                                            (static_cast<uint32_t>(data[tiff + 6]) << 8) |
                                                            static_cast<uint32_t>(data[tiff + 7]);
                    if (magic == 42) {
                        reader.parseIfd(ifd0, false, metadata);
                    }
                }
            }
        }

        pos = segmentEnd;
    }

    return metadata;
}

std::vector<std::string> availablePresetFilters() {
    return {"vivid", "landscape", "portrait", "food", "night", "document",
            "cinematic", "warm", "cool", "mono", "fade", "vintage"};
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
