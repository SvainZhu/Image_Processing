//
// Created by SvainZhu on 2022/5/23.
//

#ifndef IMAGE_PROCESSING_BASE_PROC_H
#define IMAGE_PROCESSING_BASE_PROC_H
#include <opencv2/core/mat.hpp>
#include <stdlib.h>

#include <QWidget>

using namespace cv;
using namespace std;

Mat gray_to_hist(Mat gray_image);

QVector<int> gray_to_vector(Mat gray);

Mat add_salt_noise(const Mat &src, int intensity);

Mat add_gaussian_noise(const Mat &src, int intensity);

void double_threshold(Mat &src, double low_threshold, double high_threshold);

void double_threshold_link(Mat &src, double low_threshold, double high_threshold);

void lbp_operator(Mat& src, Mat& dst, int radius, int neighbors);

double sum_of_rect(Mat& src, Rect& rect);

int get_OSTU(QVector<int> hist) {
    float u0, u1, w0, w1;
    int count, max_T;
    float dev, max_dev = 0; // deviation and max std
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum = sum + hist[i];
    }

    for (int t = 0; t < 255; t++){
        u0 = 0; count = 0;
        for (int i = 0; i <= t; i++) {
            u0 += i * hist[i];
            count += hist[i];
        }
        u0 = u0 / count;
        w0 = (float)count / sum;
        for (int i = t + 1; i < 256; i++){
            u1 += i * hist[i];
        }
        u1 = u1 / (sum - count);
        w1 = 1 - w0;
        dev = w0 * w1 * (u1 - u0) * (u1 - u0);
        if (dev > max_dev)
        {
            max_dev = dev;
            max_T = t;
        }
    }
    return max_T;
}

#endif //IMAGE_PROCESSING_BASE_PROC_H
