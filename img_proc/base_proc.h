//
// Created by SvainZhu on 2022/5/23.
//

#ifndef IMAGE_PROCESSING_BASE_PROC_H
#define IMAGE_PROCESSING_BASE_PROC_H

Mat gray_hist(Mat gray_image);

QVector<int> gray_vector(Mat gray);

Mat add_salt_noise(const src, int intensity);

Mat add_gaussian_noise(const src, int intensity);

void double_threshold(Mat &src, double low_threshold, double high_threshold);

void double_threshold_link(Mat &src, double low_threshold, double high_threshold);

int OSTU(QVector<int> hist);

void elbp(Mat& src, Mat &dst, int radius, int neighbors);

void elbp1(Mat& src, Mat &dst);

#endif //IMAGE_PROCESSING_BASE_PROC_H
