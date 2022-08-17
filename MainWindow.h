//
// Created by SvainZhu on 2022/5/19.
//

#ifndef IMAINWINDOW_H
#define MAINWINDOW_H

#pragma once
#pragma execution_character_set("utf-8")    //set the utf-8 encoder

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/objdetect.hpp>
#include <opencv2/ml.hpp>
//#include <opencv2/xfeatures2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/highgui.hpp>
//#include <opencv2/optflow/motempl.hpp>

#include <QFileDialog>
#include <QMainWindow>
#include <QWidget>

#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <limits>
#include <qmath.h>
#include <vector>
#include <fstream>

#include "base_proc.h"
using namespace cv;
using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    Mat srcImg, grayImg, saltNoiseImg, guassianNoiseImg;
private slots:
    void on_select_images_clicked();

    void on_rgb_to_gray_clicked();

    void on_gray_hist_clicked();

    void on_gray_balance_clicked();

    void on_grad_sharpen_clicked();

    void on_laplacian_sharpen_clicked();

    void on_add_salt_noise_clicked();

    void on_add_gaussian_noise_clicked();

    void on_roberts_edge_detection_clicked();

    void on_sobel_edge_detection_clicked();

    void on_prewitt_edge_detection_clicked();

    void on_laplacian_edge_detection_clicked();

    void on_krisch_edge_detection_clicked();

    void on_canny_edge_detection_clicked();

    void on_window_filter_clicked();

    void on_average_filter_clicked();

    void on_middle_filter_clicked();

    void on_gaussian_filter_clicked();

    void on_form_filter_clicked();

    void on_frame_diff_clicked();


    void on_mix_gauss_clicked();

    void on_circle_LBP_clicked();

    void on_SIFT_clicked();

    void on_haar_vertical_clicked();

    void on_haar_horizontal_clicked();

private:
    Ui::MainWindow *ui;

};

#endif //MAINWINDOW_H
