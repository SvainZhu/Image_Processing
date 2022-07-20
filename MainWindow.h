//
// Created by SvainZhu on 2022/5/19.
//

#ifndef IMAGE_PROCESSING_MAINWINDOW_H
#define IMAGE_PROCESSING_MAINWINDOW_H

#pragma once
#pragma execution_character_set("utf-8")    //set the utf-8 encoder

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/objdetect.hpp>
#include <opencv2/ml.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/highgui.hpp>
#include <opencv2/optflow/motempl.hpp>

#include <QMainWindow>

#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <limits>
#include <qmath.h>
#include <vector>
#include <fstream>

using namespace cv;
using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    Mat srcImg, grayImg, noiseImg;
private slots:
    void on_pushButton_clicked();

    void on_checkBox_clicked();

    void on_select_images_clicked();

};

#endif //IMAGE_PROCESSING_MAINWINDOW_H
