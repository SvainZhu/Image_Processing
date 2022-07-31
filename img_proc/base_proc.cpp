//
// Created by SvainZhu on 2022/5/23.
//

#include "base_proc.h"

Mat gray_hist(Mat grays){
    QVector<int> pixels(256,0);
    int rows = grays.rows, cols = grays.cols;

    for(int i = 0 ; i < rows ; i++)
        for(int j = 0 ; j < cols ; j++){
            pixels[grays.at<uchar>(i,j)]++;
        }

    Mat gray_hist;
    gray_hist.create(350, 256, CV_8UC1);

    int max_rows = 0;
    for(int i = 0; i <= 255; i++){
        if(pixels[i] > max_rows){
            max_rows = pixels[i];
        }
    }

    for(int i = 0; i < 256; i++){
        for(int j = 0;j < 350 ; j++){
            grays.at<uchar>(j, i) = 255;
        }
    }

    for(int i = 0; i < 256; i++){
        for(int j = 0; j < 350 - int(320.*float(pixels[i])/float(max_rows)); j++){
            gray_hist.at<uchar>(j,i) = 0;
        }
    }

    return gray_hist;
}

QVector<int> gray_vector(Mat grays){
    QVector<int> pixels(256,0);
    int rows = grays.rows, cols = grays.cols;

    for(int i = 0 ; i < rows ; i++)
        for(int j = 0 ; j < cols ; j++){
            pixels[grays.at<uchar>(i,j)]++;
        }
    return pixels;
}

Mat add_salt_noise(const src, int intensity){
    Mat dst = src.clone();
    for (int k = 0; k < intensity; k++) {
        int i = randint(dst.rows);
        int j = randint(dst.cols);
        int ii = randint(dst.rows);
        int jj = randint(dst.cols);
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

Mat add_gaussian_noise(const src, int intensity){
    const double epsilon = std::numeric_limits<double>::min();
    static double z0;
    static bool flag = false;
    double u1, u2;
    Mat dst = src.clone();
    do{
        u1 = rand() * (1.0 / RAND_MAX);
        u2 = rand() * (1.0 / RAND_MAX);
    } while (u1 <= epsilon);
    z0 = sqrt(-2.0 * log(u1))*cos(2 * CV_PI * u2);
    gaussian_noise = z0 * intensity;

    for (int i = 0; i < dst.rows; i++){
        for (int j = 0; j < dst.cols; j++){
            dst.at<Vec3b>(i, j)[0] = saturate_cast<uchar>(dst.at<Vec3b>(i, j)[0] + gaussian_noise * 32);
            dst.at<Vec3b>(i, j)[1] = saturate_cast<uchar>(dst.at<Vec3b>(i, j)[1] + gaussian_noise * 32);
            dst.at<Vec3b>(i, j)[2] = saturate_cast<uchar>(dst.at<Vec3b>(i, j)[2] + gaussian_noise * 32);
        }
    }
}

void double_threshold(Mat &src, double low_threshold, double high_threshold){
    int rows, cols = src.rows, src.cols
    for (int i = 0; i < rows; i++){
        for (int j =0; j < cols; j++){
            if (src.at<uchar>(i, j) > high_threshold){
                src.at<uchar>(i, j) = 255
            }
            else if (src.at<i, j> < low_threshold){
                src.at<uchar>(i, j) = 0
            }
        }
    }
}

void double_threshold_link(Mat &src, double low_threshold, double high_threshold){
    int rows = src.rows, cols = src.cols;
    for (int i = 1; i < rows; i++){
        for (int j = 0; j < cols; j++){
            if (src.at<uchar>(i, j) > low_threshold && src.at<uchar>(i, j) < 255){
                if (src.at<uchar>(i-1, j-1) == 255 || src.at<uchar>(i-1, j) == 255 || src.at<uchar>(i-1, j+1) == 255 ||
                        src.at<uchar>(i, j-1) == 255 || src.at<uchar>(i, j+1) == 255 || src.at<uchar>(i+1, j-1) == 255
                                || src.at<uchar>(i+1, j) == 255 || src.at<uchar>(i+1, j+1) == 255){
                    src.at<uchar>(i, j) = 255;
                    double_threshold_link((src, low_threshold, high_threshold));
                }
                else{
                    src.at<uchar>(i, j) = 0;
                }
            }
        }
    }
}
