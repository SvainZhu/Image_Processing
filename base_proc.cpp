//
// Created by SvainZhu on 2022/5/23.
//

#include "base_proc.h"

#include <algorithm>
#include <cmath>
#include <limits>

Mat gray_to_hist(Mat grays){
    if (grays.empty()) {
        return Mat();
    }
    if (grays.channels() != 1) {
        cvtColor(grays, grays, COLOR_BGR2GRAY);
    }
    QVector<int> pixels(256,0);
    int rows = grays.rows, cols = grays.cols;

    for(int i = 0 ; i < rows ; i++)
        for(int j = 0 ; j < cols ; j++){
            pixels[grays.at<uchar>(i,j)]++;
        }

    Mat gray_hist;
    gray_hist = Mat::zeros(350, 256, CV_8UC1);

    int max_rows = 0;
    for(int i = 0; i <= 255; i++){
        if(pixels[i] > max_rows){
            max_rows = pixels[i];
        }
    }

    if (max_rows == 0) {
        return gray_hist;
    }

    for(int i = 0; i < 256; i++){
        for(int j = 0;j < 350 ; j++){
            gray_hist.at<uchar>(j, i) = 255;
        }
    }

    for(int i = 0; i < 256; i++){
        for(int j = 0; j < 350 - int(320.*float(pixels[i])/float(max_rows)); j++){
            gray_hist.at<uchar>(j,i) = 0;
        }
    }

    return gray_hist;
}

QVector<int> gray_to_vector(Mat grays){
    if (grays.empty()) {
        return QVector<int>(256, 0);
    }
    if (grays.channels() != 1) {
        cvtColor(grays, grays, COLOR_BGR2GRAY);
    }
    QVector<int> pixels(256,0);
    int rows = grays.rows, cols = grays.cols;

    for(int i = 0 ; i < rows ; i++)
        for(int j = 0 ; j < cols ; j++){
            pixels[grays.at<uchar>(i,j)]++;
        }
    return pixels;
}

Mat add_salt_noise(const Mat &src, int intensity){
    if (src.empty() || intensity <= 0) {
        return src.clone();
    }
    Mat dst = src.clone();
    for (int k = 0; k < intensity; k++) {
        int i = rand() % dst.rows;
        int j = rand() % dst.cols;
        int ii = rand() % dst.rows;
        int jj = rand() % dst.cols;
        if (dst.channels() == 1) {
            dst.at<uchar>(i, j) = 255;
            dst.at<uchar>(ii, jj) = 0;
        } else {
            dst.at<Vec3b>(i, j)[0] = 255;
            dst.at<Vec3b>(i, j)[1] = 255;
            dst.at<Vec3b>(i, j)[2] = 255;
            dst.at<Vec3b>(ii, jj)[0] = 0;
            dst.at<Vec3b>(ii, jj)[1] = 0;
            dst.at<Vec3b>(ii, jj)[2] = 0;
        }
    }
    return dst;
}

Mat add_gaussian_noise(const Mat &src, int intensity){
    if (src.empty() || intensity <= 0) {
        return src.clone();
    }
    const double epsilon = std::numeric_limits<double>::min();
    double u1, u2, gaussian_noise;
    Mat dst = src.clone();

    for (int i = 0; i < dst.rows; i++){
        for (int j = 0; j < dst.cols; j++){
            do{
                u1 = rand() * (1.0 / RAND_MAX);
                u2 = rand() * (1.0 / RAND_MAX);
            } while (u1 <= epsilon);
            gaussian_noise = sqrt(-2.0 * log(u1)) * cos(2 * CV_PI * u2) * intensity;

            if (dst.channels() == 1) {
                dst.at<uchar>(i, j) = saturate_cast<uchar>(dst.at<uchar>(i, j) + gaussian_noise);
            } else {
                for (int c = 0; c < dst.channels(); c++) {
                    dst.ptr<uchar>(i)[j * dst.channels() + c] =
                            saturate_cast<uchar>(dst.ptr<uchar>(i)[j * dst.channels() + c] + gaussian_noise);
                }
            }
        }
    }
    return dst;
}

void double_threshold(Mat &src, double low_threshold, double high_threshold){
    if (src.empty()) {
        return;
    }
    int rows = src.rows, cols = src.cols;
    for (int i = 0; i < rows; i++){
        for (int j =0; j < cols; j++){
            if (src.at<uchar>(i, j) > high_threshold){
                src.at<uchar>(i, j) = 255;
            }
            else if (src.at<uchar>(i, j) < low_threshold){
                src.at<uchar>(i, j) = 0;
            }
        }
    }
}

void double_threshold_link(Mat &src, double low_threshold, double high_threshold){
    if (src.empty()) {
        return;
    }
    double_threshold(src, low_threshold, high_threshold);
    int rows = src.rows, cols = src.cols;
    Mat linked = src.clone();
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 1; i < rows - 1; i++){
            for (int j = 1; j < cols - 1; j++){
                if (linked.at<uchar>(i, j) > low_threshold && linked.at<uchar>(i, j) < high_threshold){
                    bool has_strong_neighbor = false;
                    for (int y = -1; y <= 1; y++) {
                        for (int x = -1; x <= 1; x++) {
                            if (linked.at<uchar>(i + y, j + x) == 255) {
                                has_strong_neighbor = true;
                            }
                        }
                    }
                    if (has_strong_neighbor) {
                        linked.at<uchar>(i, j) = 255;
                        changed = true;
                    }
                }
            }
        }
    }
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (linked.at<uchar>(i, j) != 255) {
                linked.at<uchar>(i, j) = 0;
            }
        }
    }
    src = linked;
}

void lbp_operator(Mat& src, Mat& dst, int radius, int neighbors){
    if (src.empty() || radius <= 0 || neighbors <= 0 ||
        src.rows <= 2 * radius || src.cols <= 2 * radius) {
        return;
    }
    if (src.channels() != 1) {
        cvtColor(src, src, COLOR_BGR2GRAY);
    }
    dst = Mat::zeros(src.rows - 2 * radius, src.cols - 2 * radius, CV_8UC1);
    for (int n = 0; n < neighbors; n++){
        float x = static_cast<float>(-radius * sin(2.0 * CV_PI*n / static_cast<float>(neighbors)));
        float y = static_cast<float>(radius * cos(2.0 * CV_PI*n / static_cast<float>(neighbors)));

        int floor_x = static_cast<int>(floor(x));
        int floor_y = static_cast<int>(floor(y));
        int ceil_x = static_cast<int>(ceil(x));
        int ceil_y = static_cast<int>(ceil(y));

        float decimal_x = x - floor_x;
        float decimal_y = y - floor_y;

        float w1 = (1 - decimal_x) * (1 - decimal_y);
        float w2 = decimal_x * (1 - decimal_y);
        float w3 = (1 - decimal_x) * decimal_y;
        float w4 = decimal_x * decimal_y;

        for (int i = radius; i < src.rows - radius; i++){
            for (int j = radius; j < src.cols - radius; j++){
                float t = static_cast<float>(w1 * src.at<uchar>(i+floor_y, j+floor_x) + w2 * src.at<uchar>(i+floor_y, j+ceil_x)
                                            + w3 * src.at<uchar>(i+ceil_y, j+floor_x) + w4 * src.at<uchar>(i+ceil_y, j+ceil_x));
                dst.at<uchar>(i-radius, j-radius) += ((t > src.at<uchar>(i, j)) || (abs(t-src.at<uchar>(i, j)) < numeric_limits<float>::epsilon())) << n;
            }
        }
    }
}

double mat_value(const Mat &src, int y, int x) {
    y = std::max(0, std::min(y, src.rows - 1));
    x = std::max(0, std::min(x, src.cols - 1));
    switch (src.depth()) {
        case CV_8U:
            return src.at<uchar>(y, x);
        case CV_16U:
            return src.at<unsigned short>(y, x);
        case CV_16S:
            return src.at<short>(y, x);
        case CV_32S:
            return src.at<int>(y, x);
        case CV_32F:
            return src.at<float>(y, x);
        case CV_64F:
            return src.at<double>(y, x);
        default:
            return 0.0;
    }
}

double sum_of_rect(const Mat& src, const Rect& rect){
    if (src.empty()) {
        return 0.0;
    }
    int x = rect.x, y = rect.y, width = rect.width, height = rect.height;
    return mat_value(src, y, x) + mat_value(src, y+height, x+width)
           - mat_value(src, y+height, x) - mat_value(src, y, x+width);
}

int get_OSTU(const QVector<int> &hist) {
    if (hist.size() < 256) {
        return 0;
    }

    double total = 0.0;
    double sum = 0.0;
    for (int i = 0; i < 256; i++) {
        total += hist[i];
        sum += i * hist[i];
    }
    if (total <= 0.0) {
        return 0;
    }

    double sum_b = 0.0;
    double weight_b = 0.0;
    double max_variance = -1.0;
    int threshold = 0;

    for (int t = 0; t < 256; t++) {
        weight_b += hist[t];
        if (weight_b == 0.0) {
            continue;
        }
        double weight_f = total - weight_b;
        if (weight_f == 0.0) {
            break;
        }

        sum_b += t * hist[t];
        double mean_b = sum_b / weight_b;
        double mean_f = (sum - sum_b) / weight_f;
        double between_variance = weight_b * weight_f * (mean_b - mean_f) * (mean_b - mean_f);

        if (between_variance > max_variance) {
            max_variance = between_variance;
            threshold = t;
        }
    }
    return threshold;
}
