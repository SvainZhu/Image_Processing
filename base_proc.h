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

#endif //IMAGE_PROCESSING_BASE_PROC_H
