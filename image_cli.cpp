#include "image_ops.h"

#include <opencv2/imgcodecs.hpp>

#include <cstdlib>
#include <cctype>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using Options = std::map<std::string, std::string>;

void printUsage() {
    std::cout
            << "Image Processing CLI\n"
            << "Usage:\n"
            << "  image_cli <input> <output> <operation> [key=value ...]\n\n"
            << "  image_cli <input> metadata\n\n"
            << "Operations:\n"
            << "  gray | grayscale\n"
            << "  resize width=<px> height=<px> keep_aspect=true|false\n"
            << "  crop x=<px> y=<px> width=<px> height=<px>\n"
            << "  rotate angle=<degrees> [scale=1.0]\n"
            << "  flip code=-1|0|1\n"
            << "  histogram [width=512 height=300]\n"
            << "  equalize\n"
            << "  clahe [clip=2.0 tile=8]\n"
            << "  brightness [alpha=1.0 beta=0]\n"
            << "  gamma gamma=<value>\n"
            << "  white_balance\n"
            << "  saturation [factor=1.0]\n"
            << "  vibrance [amount=0.25]\n"
            << "  temperature [temperature=0 tint=0]\n"
            << "  curve points=0:0,128:140,255:255 [channel=all|r|g|b]\n"
            << "  filter preset=vivid|portrait|landscape|food|night|document|cinematic|warm|cool|mono|fade|vintage [intensity=1]\n"
            << "  auto_enhance [strength=1]\n"
            << "  metadata\n"
            << "  threshold value=<gray> [max=255]\n"
            << "  otsu\n"
            << "  adaptive [block=11 c=2 gaussian=true]\n"
            << "  blur [ksize=3]\n"
            << "  gaussian_blur [ksize=5 sigma=0]\n"
            << "  median [ksize=3]\n"
            << "  bilateral [diameter=9 sigma_color=75 sigma_space=75]\n"
            << "  nlm_denoise [h=10 template=7 search=21]\n"
            << "  morphology type=erode|dilate|open|close|gradient|tophat|blackhat [ksize=3 iterations=1]\n"
            << "  canny [low=80 high=160 aperture=3]\n"
            << "  auto_canny [sigma=0.33 aperture=3]\n"
            << "  sobel [dx=1 dy=0 ksize=3]\n"
            << "  laplacian [ksize=3]\n"
            << "  gradient [ksize=3 scharr=false]\n"
            << "  contours [value=0 otsu=true thickness=2]\n"
            << "  components [min_area=0]\n"
            << "  distance_transform\n"
            << "  watershed\n"
            << "  hough_lines [rho=1 theta=0.0174533 threshold=80 min_length=30 max_gap=10]\n"
            << "  hough_circles [dp=1.2 min_dist=40 param1=100 param2=30 min_radius=0 max_radius=0]\n"
            << "  template_match template=<path> [method=ccoeff_normed]\n"
            << "  orb [features=500]\n"
            << "  inpaint mask=<path> [radius=3]\n"
            << "  pyrdown [levels=1]\n"
            << "  pyrup [levels=1]\n"
            << "  affine a00=<v> a01=<v> a02=<v> a10=<v> a11=<v> a12=<v> [width=0 height=0]\n"
            << "  sharpen\n"
            << "  unsharp [amount=1.0 ksize=5 sigma=1]\n"
            << "  salt_pepper [amount=0.01 seed=0]\n"
            << "  gaussian_noise [mean=0 stddev=10 seed=0]\n"
            << "  colormap map=jet|viridis|turbo|hot|... \n"
            << "  colorspace code=bgr2gray|bgr2hsv|bgr2lab|bgr2rgb|...\n";
}

Options parseOptions(int argc, char **argv, int start) {
    Options options;
    for (int i = start; i < argc; ++i) {
        const std::string item(argv[i]);
        const std::size_t pos = item.find('=');
        if (pos == std::string::npos) {
            throw std::invalid_argument("Option must use key=value format: " + item);
        }
        options[item.substr(0, pos)] = item.substr(pos + 1);
    }
    return options;
}

std::string getString(const Options &options, const std::string &key,
                      const std::string &defaultValue) {
    const auto it = options.find(key);
    return it == options.end() ? defaultValue : it->second;
}

int getInt(const Options &options, const std::string &key, int defaultValue) {
    const auto it = options.find(key);
    return it == options.end() ? defaultValue : std::stoi(it->second);
}

double getDouble(const Options &options, const std::string &key, double defaultValue) {
    const auto it = options.find(key);
    return it == options.end() ? defaultValue : std::stod(it->second);
}

bool getBool(const Options &options, const std::string &key, bool defaultValue) {
    const auto it = options.find(key);
    if (it == options.end()) {
        return defaultValue;
    }
    return it->second == "true" || it->second == "1" || it->second == "yes";
}

int requireInt(const Options &options, const std::string &key) {
    const auto it = options.find(key);
    if (it == options.end()) {
        throw std::invalid_argument("Missing required option: " + key);
    }
    return std::stoi(it->second);
}

double requireDouble(const Options &options, const std::string &key) {
    const auto it = options.find(key);
    if (it == options.end()) {
        throw std::invalid_argument("Missing required option: " + key);
    }
    return std::stod(it->second);
}

std::string requireString(const Options &options, const std::string &key) {
    const auto it = options.find(key);
    if (it == options.end() || it->second.empty()) {
        throw std::invalid_argument("Missing required option: " + key);
    }
    return it->second;
}

cv::Mat loadOptionImage(const Options &options, const std::string &key) {
    const std::string path = requireString(options, key);
    cv::Mat image = cv::imread(path, cv::IMREAD_UNCHANGED);
    if (image.empty()) {
        throw std::runtime_error("Cannot read " + key + " image: " + path);
    }
    return image;
}

std::vector<cv::Point2f> parseCurvePoints(const std::string &text) {
    std::vector<cv::Point2f> points;
    std::stringstream stream(text);
    std::string item;
    while (std::getline(stream, item, ',')) {
        const std::size_t pos = item.find(':');
        if (pos == std::string::npos) {
            throw std::invalid_argument("Curve point must use x:y format: " + item);
        }
        const float x = std::stof(item.substr(0, pos));
        const float y = std::stof(item.substr(pos + 1));
        points.push_back(cv::Point2f(x, y));
    }
    if (points.size() < 2) {
        throw std::invalid_argument("Curve points require at least two x:y pairs");
    }
    return points;
}

void printMetadata(const std::map<std::string, std::string> &metadata) {
    for (const auto &item : metadata) {
        std::cout << item.first << "=" << item.second << '\n';
    }
}

std::string normalizeOp(std::string op) {
    for (char &ch : op) {
        if (ch == '-') {
            ch = '_';
        } else {
            ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        }
    }
    return op;
}

cv::Mat runOperation(const cv::Mat &image, const std::string &operation,
                     const Options &options) {
    const std::string op = normalizeOp(operation);

    if (op == "gray" || op == "grayscale") {
        return imgproc::toGray(image);
    }
    if (op == "resize") {
        return imgproc::resizeImage(image, getInt(options, "width", image.cols),
                                    getInt(options, "height", image.rows),
                                    getBool(options, "keep_aspect", false));
    }
    if (op == "crop") {
        return imgproc::cropImage(image, cv::Rect(requireInt(options, "x"),
                                                 requireInt(options, "y"),
                                                 requireInt(options, "width"),
                                                 requireInt(options, "height")));
    }
    if (op == "rotate") {
        return imgproc::rotateImage(image, getDouble(options, "angle", 0.0),
                                    getDouble(options, "scale", 1.0));
    }
    if (op == "flip") {
        return imgproc::flipImage(image, getInt(options, "code", 1));
    }
    if (op == "hist" || op == "histogram") {
        return imgproc::histogramImage(image, getInt(options, "width", 512),
                                       getInt(options, "height", 300));
    }
    if (op == "equalize") {
        return imgproc::equalizeHistogram(image);
    }
    if (op == "clahe") {
        return imgproc::claheEqualize(image, getDouble(options, "clip", 2.0),
                                      getInt(options, "tile", 8));
    }
    if (op == "brightness" || op == "contrast") {
        return imgproc::adjustBrightnessContrast(image, getDouble(options, "alpha", 1.0),
                                                 getDouble(options, "beta", 0.0));
    }
    if (op == "gamma") {
        return imgproc::gammaCorrection(image, getDouble(options, "gamma", 1.0));
    }
    if (op == "white_balance" || op == "gray_world") {
        return imgproc::grayWorldWhiteBalance(image);
    }
    if (op == "saturation") {
        return imgproc::adjustSaturation(image, getDouble(options, "factor", 1.0));
    }
    if (op == "vibrance" || op == "vividness") {
        return imgproc::adjustVibrance(image, getDouble(options, "amount", 0.25));
    }
    if (op == "temperature" || op == "color_balance") {
        return imgproc::adjustTemperatureTint(image,
                                              getDouble(options, "temperature", 0.0),
                                              getDouble(options, "tint", 0.0));
    }
    if (op == "curve" || op == "tone_curve") {
        return imgproc::applyToneCurve(
                image,
                parseCurvePoints(requireString(options, "points")),
                getString(options, "channel", "all"));
    }
    if (op == "filter" || op == "preset") {
        return imgproc::applyPresetFilter(image,
                                          getString(options, "preset", "vivid"),
                                          getDouble(options, "intensity", 1.0));
    }
    if (op == "auto_enhance" || op == "auto_adjust" || op == "auto") {
        return imgproc::autoEnhance(image, getDouble(options, "strength", 1.0));
    }
    if (op == "threshold") {
        return imgproc::thresholdBinary(image, getDouble(options, "value", 128.0),
                                        getDouble(options, "max", 255.0));
    }
    if (op == "otsu") {
        return imgproc::thresholdOtsu(image);
    }
    if (op == "adaptive") {
        return imgproc::adaptiveThresholdImage(image, getInt(options, "block", 11),
                                               getDouble(options, "c", 2.0),
                                               getBool(options, "gaussian", true));
    }
    if (op == "blur" || op == "mean_blur") {
        return imgproc::meanBlurImage(image, getInt(options, "ksize", 3));
    }
    if (op == "gaussian_blur") {
        return imgproc::gaussianBlurImage(image, getInt(options, "ksize", 5),
                                          getDouble(options, "sigma", 0.0));
    }
    if (op == "median" || op == "median_blur") {
        return imgproc::medianBlurImage(image, getInt(options, "ksize", 3));
    }
    if (op == "bilateral") {
        return imgproc::bilateralFilterImage(image, getInt(options, "diameter", 9),
                                             getDouble(options, "sigma_color", 75.0),
                                             getDouble(options, "sigma_space", 75.0));
    }
    if (op == "nlm_denoise" || op == "denoise") {
        return imgproc::denoiseNlMeans(image, static_cast<float>(getDouble(options, "h", 10.0)),
                                       getInt(options, "template", 7),
                                       getInt(options, "search", 21));
    }
    if (op == "morph" || op == "morphology") {
        return imgproc::morphologyImage(image,
                                        imgproc::parseMorphType(getString(options, "type", "open")),
                                        getInt(options, "ksize", 3),
                                        getInt(options, "iterations", 1));
    }
    if (op == "canny") {
        return imgproc::cannyEdge(image, getDouble(options, "low", 80.0),
                                  getDouble(options, "high", 160.0),
                                  getInt(options, "aperture", 3));
    }
    if (op == "auto_canny") {
        return imgproc::autoCannyEdge(image, getDouble(options, "sigma", 0.33),
                                      getInt(options, "aperture", 3));
    }
    if (op == "sobel") {
        return imgproc::sobelEdge(image, getInt(options, "dx", 1),
                                  getInt(options, "dy", 0),
                                  getInt(options, "ksize", 3));
    }
    if (op == "laplacian") {
        return imgproc::laplacianEdge(image, getInt(options, "ksize", 3));
    }
    if (op == "gradient") {
        return imgproc::gradientMagnitude(image, getInt(options, "ksize", 3),
                                          getBool(options, "scharr", false));
    }
    if (op == "contours") {
        return imgproc::contoursImage(image, getDouble(options, "value", 0.0),
                                      getBool(options, "otsu", true),
                                      getInt(options, "thickness", 2));
    }
    if (op == "components" || op == "connected_components") {
        return imgproc::connectedComponentsImage(image, getInt(options, "min_area", 0));
    }
    if (op == "distance_transform") {
        return imgproc::distanceTransformImage(image);
    }
    if (op == "watershed") {
        return imgproc::watershedSegmentation(image);
    }
    if (op == "hough_lines") {
        return imgproc::houghLinesImage(image, getDouble(options, "rho", 1.0),
                                        getDouble(options, "theta", CV_PI / 180.0),
                                        getInt(options, "threshold", 80),
                                        getDouble(options, "min_length", 30.0),
                                        getDouble(options, "max_gap", 10.0));
    }
    if (op == "hough_circles") {
        return imgproc::houghCirclesImage(image, getDouble(options, "dp", 1.2),
                                          getDouble(options, "min_dist", 40.0),
                                          getDouble(options, "param1", 100.0),
                                          getDouble(options, "param2", 30.0),
                                          getInt(options, "min_radius", 0),
                                          getInt(options, "max_radius", 0));
    }
    if (op == "template_match") {
        return imgproc::templateMatchImage(
                image,
                loadOptionImage(options, "template"),
                imgproc::parseTemplateMatchMethod(getString(options, "method", "ccoeff_normed")));
    }
    if (op == "orb") {
        return imgproc::drawOrbKeypoints(image, getInt(options, "features", 500));
    }
    if (op == "inpaint") {
        return imgproc::inpaintImage(image, loadOptionImage(options, "mask"),
                                     getDouble(options, "radius", 3.0));
    }
    if (op == "pyrdown") {
        return imgproc::pyramidDown(image, getInt(options, "levels", 1));
    }
    if (op == "pyrup") {
        return imgproc::pyramidUp(image, getInt(options, "levels", 1));
    }
    if (op == "affine") {
        return imgproc::affineTransform(image,
                                        requireDouble(options, "a00"),
                                        requireDouble(options, "a01"),
                                        requireDouble(options, "a02"),
                                        requireDouble(options, "a10"),
                                        requireDouble(options, "a11"),
                                        requireDouble(options, "a12"),
                                        getInt(options, "width", 0),
                                        getInt(options, "height", 0));
    }
    if (op == "sharpen") {
        return imgproc::sharpen(image);
    }
    if (op == "unsharp") {
        return imgproc::unsharpMask(image, getDouble(options, "amount", 1.0),
                                    getInt(options, "ksize", 5),
                                    getDouble(options, "sigma", 1.0));
    }
    if (op == "salt_pepper") {
        return imgproc::addSaltPepperNoise(image, getDouble(options, "amount", 0.01),
                                           static_cast<unsigned int>(getInt(options, "seed", 0)));
    }
    if (op == "gaussian_noise") {
        return imgproc::addGaussianNoise(image, getDouble(options, "mean", 0.0),
                                         getDouble(options, "stddev", 10.0),
                                         static_cast<unsigned int>(getInt(options, "seed", 0)));
    }
    if (op == "colormap") {
        return imgproc::applyColorMapImage(image,
                                           imgproc::parseColorMap(getString(options, "map", "jet")));
    }
    if (op == "colorspace") {
        return imgproc::convertColor(image,
                                     imgproc::parseColorConversion(getString(options, "code", "bgr2gray")));
    }

    throw std::invalid_argument("Unknown operation: " + operation);
}

}  // namespace

int main(int argc, char **argv) {
    if (argc < 2 || std::string(argv[1]) == "--help" || std::string(argv[1]) == "help") {
        printUsage();
        return argc < 2 ? 1 : 0;
    }
    if (argc >= 3) {
        const std::string mode = normalizeOp(argv[2]);
        if (mode == "metadata" || mode == "exif") {
            try {
                printMetadata(imgproc::readImageMetadata(argv[1]));
                return 0;
            } catch (const std::exception &error) {
                std::cerr << "Error: " << error.what() << std::endl;
                return 2;
            }
        }
    }
    if (argc < 4) {
        printUsage();
        return 1;
    }

    try {
        const std::string inputPath(argv[1]);
        const std::string outputPath(argv[2]);
        const std::string operation(argv[3]);
        const Options options = parseOptions(argc, argv, 4);

        const cv::Mat image = cv::imread(inputPath, cv::IMREAD_UNCHANGED);
        if (image.empty()) {
            throw std::runtime_error("Cannot read input image: " + inputPath);
        }

        const cv::Mat result = runOperation(image, operation, options);
        if (!cv::imwrite(outputPath, result)) {
            throw std::runtime_error("Cannot write output image: " + outputPath);
        }

        std::cout << "Wrote " << outputPath << std::endl;
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "Error: " << error.what() << std::endl;
        return 2;
    }
}
