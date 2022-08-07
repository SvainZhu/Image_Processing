//
// Created by SvainZhu on 2022/5/19.
//
#include "mainwindow.h"
#include "ui_mainwindow.h"

// init main window
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    Ui->setupUi(this);
}

//deploy the class of MainWindow
MainWindow::~MainWindow(){
    delete Ui;
}

void MainWindow::on_pushButton_clicked(){
    QString img_name = QFileDialog::getOpenFileName(this, tr(""), "../../../../test_image", "files(*)");
    img_name = img_name.toStdString();
    srcImg = imread(img_name);
    cvtColor(srcImg, grayImg, CV_BRG2GRAY);

    Mat temp;
    QImage QTemp;
    cvtColor(srcImg, temp, CV_BGR2RGB);
    QTemp = QImage((const unsigned char *)(temp.data), temp.cols, temp.rows, temp.step, QImage::Format_RGB888);

    ui->label->setPixmap(QPixmap::fromImage(QTemp));
    QTemp = QTemp.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label->setScaledContents(true);
    ui->label->resize(QTemp.size);
    ui->label->show;

}

void on_select_images_clicked(){
    int rows = srcImg.rows, cols = srcImg.cols;
    grayImg.create(rows, cols, CV_8UC1);
    QImage QTemp;

    for (int i = 0; i < rows; i++){
        for (int j = 0; j < cols; j++){
            grayImg.at<uchar>(i, j) = (int) 0.11 * srcImg.at<Vec3b>(i, j)[0]
                                        + 0.59 * srcImg.at<Vec3b>(i, j)[1]
                                        + 0.3 * srcImg.at<Vec3b>(i, j)[2];
        }
    }

    QTemp = QImage((const uchar*)(grayImg.data), grayImg.cols, grayImg.rows, grayImg.cols * grayImg.channels(), QImage::Format_Indexed8);
    ui->label_1->setPixmap(QPixmap::fromImage(QTemp));
    QTemp = QTemp.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_1->setScaledContents(true);
    ui->label_1->resize(QTemp.size);
    ui->label_1->show();

}

void MainWindow::on_gray_hist_clicked(){
    QImage QTemp;
    QTemp = QImage((const uchar*)(grayImg.data), grayImg.cols, grayImg.rows, grayImg.cols * grayImg.channels(), QImage::Format_Indexed8);
    ui->label_1->setPixmap(QPixmap::fromImage(QTemp));
    QTemp = QTemp.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setScaledContents(true);
    ui->label_2->resize(QTemp.size);
    ui->label_2->show();

    Mat gray_hist;
    gray_hist = gray_to_hist(grayImg);
    imshow("gray histogram", gray_hist);
    waitKey(0);
    cv::destoryWindow("gray histogram");
    waitKey(1);
}

void MainWindow::on_gray_balance_clicked(){
    Mat balance, gray_img;
    gray_img.create(srcImg.rows, srcImg.cols, CV_8UC1);
    balance.create(srcImg.rows, scrImg.cols, CV_8UC1);

    QImage QTemp;
    QVector<int> pixel(256, 0);
    QVector<float> pixel_gray(256.,0.);
    float sum = 0;

    for (int i = 0; i < grayImg.rows; i++){
        for (int j = 0; j < grayImg.cols; j++){
            pixel[grayImg.at<uchar>(i, j)]++;
        }
    }

    for (int i = 0; i < pixel.size(), i++){
        sum += pixel[i]
    }

    for (int i = 0; i < 256; i++){
        float num = 0.;
        for (int j = 0; j <= i; j++){
            num += pixel[j];
        }
        pixel_gray[i] = 255 * num / sum;
    }

    for (int i = 0; i < srcImg.rows; i++){
        for (int j = 0; j < srcImg.cols; j++){
            balance.at<uchar>(i, j) = pixel_gray[grayImg.at<uchar>(i, j)];
        }
    }

    gray_img = balance;

    QTemp = QImage((const uchar*)(gray_img.data), gray_img.cols, gray_img.rows, gray_img.cols * gray_img.channels(), QImage::Format_Indexed8);
    ui->label_3->setPixmap(QPixmap::fromImage(QTemp));
    QTemp = QTemp.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_3->setScaledContents(true);
    ui->label_3->resize(QTemp.size);
    ui->label_3->show();

}

void MainWindow::on_gray_sharpen_clicked(){
    Mat grad, gray_img;
    QImage QTemp1, QTemp2;
    grad.create(srcImg.rows, srcImg.cols, CV_8UC1);
    gray_img.create(srcImg.rows, srcImg.cols, CV_8UC1);
    for (int i = 0; i < gray_img.rows - 1; i++){
        for (int j = 0; j < gray_img.cols - 1; j++){
            grad.at<uchar>(i, j) = saturate_cast<uchar>(max(fabs(grayImg.at<uchar>(i+1, j) - grayImg<uchar>(i, j)),
                                                            fabs(grayImg.at<uchar>(i, j+1) - grayImg<uchar>(i, j))));
            gray_img.at<uchar>(i, j) = saturate_cast<uchar>(grayImg.at<uchar>(i, j) - grad<uchar>(i, j))
        }
    }

    QTemp1 = QImage((const uchar*)(gray_img.data), gray_img.cols, gray_img.rows, gray_img.cols * gray_img.channels(), QImage::Format_Indexed8);
    ui->label_3->setPixmap(QPixmap::fromImage(QTemp1));
    QTemp1 = QTemp1.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_3->setScaledContents(true);
    ui->label_3->resize(QTemp1.size);
    ui->label_3->show();

    QTemp2 = QImage((const uchar*)(grad.data), grad.cols, grad.rows, grad.cols * grad.channels(), QImage::Format_Indexed8);
    ui->label_2->setPixmap(QPixmap::fromImage(QTemp2));
    QTemp2 = QTemp2.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setScaledContents(true);
    ui->label_2->resize(QTemp2.size);
    ui->label_2->show();


}

void MainWindow::on_laplacian_sharpen_clicked(){
    Mat grad, gray_img;
    QImage QTemp1, QTemp2;
    grad.create(srcImg.rows, srcImg.cols, CV_8UC1);
    gray_img.create(srcImg.rows, srcImg.cols, CV_8UC1);
    for (int i = 1; i < gray_img.rows - 1; i++){
        for (int j = 1; j < gray_img.cols - 1; j++){
            grad.at<uchar>(i, j) = saturate_cast<uchar>(grayImg.at<uchar>(i-1, j) - 4 * grayImg.at<uchar>(i, j)
                                                        + grayImg.at<uchar>(i+1, j) + grayImg.at<uchar>(i, j-1)
                                                        + grayImg.at<uchar>(i, j+1));
            gray_img.at<uchar>(i, j) = saturate_cast<uchar>(- grayImg.at<uchar>(i-1, j) + 5 * grayImg.at<uchar>(i, j)
                                                            - grayImg.at<uchar>(i+1, j) - grayImg.at<uchar>(i, j-1)
                                                            - grayImg.at<uchar>(i, j+1))
        }
    }

    QTemp1 = QImage((const uchar*)(gray_img.data), gray_img.cols, gray_img.rows, gray_img.cols * gray_img.channels(), QImage::Format_Indexed8);
    ui->label_3->setPixmap(QPixmap::fromImage(QTemp1));
    QTemp1 = QTemp1.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_3->setScaledContents(true);
    ui->label_3->resize(QTemp1.size);
    ui->label_3->show();

    QTemp2 = QImage((const uchar*)(grad.data), grad.cols, grad.rows, grad.cols * grad.channels(), QImage::Format_Indexed8);
    Ui->label_2->setPixmap(QPixmap::fromImage(QTemp2));
    QTemp2 = QTemp2.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setScaledContents(true);
    ui->label_2->resize(QTemp2.size);
    ui->label_2->show();

}

void MainWindow::on_add_salt_noise_clicked(){
    Mat salt_noise, temp;
    QImage QTemp;
    salt_noise.create(srcImg.rows, srcImg.cols, CV_8UC3);
    salt_noise = add_salt_noise(srcImg, 500);
    cvtColor(salt_noise, temp, CV_BGR2RGB);

    saltNoiseImg = temp.clone();
    QTemp = QImage((const uchar*)(temp.data), temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
    ui->label_2->setPixmap(QPixmap::fromImage(QTemp));
    QTemp = QTemp.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setScaledContents(true);
    ui->label_2->resize(QTemp.size());
    ui->label_2->show();


}

void MainWindow::on_add_guassian_noise_clicked(){
    Mat guassian_noise, temp;
    QImage QTemp;
    guassian_noise.create(srcImg.rows, srcImg.cols, CV_8UC3);
    guassian_noise = add_salt_noise(srcImg, 500);
    cvtColor(guassian_noise, temp, CV_BGR2RGB);

    guassianNoiseImg = temp.clone();
    QTemp = QImage((const uchar*)(temp.data), temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
    ui->label_2->setPixmap(QPixmap::fromImage(QTemp));
    QTemp = QTemp.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setScaledContents(true);
    ui->label_2->resize(QTemp.size());
    ui->label_2->show();

}

void MainWindow::on_roberts_edge_detection_clicked(){
    Mat grad, gray_img;
    QImage QTemp1, QTemp2;
    grad.create(grayImg.rows, grayImg.cols, CV_8UC1);
    gray_img.create(grayImg.rows, grayImg.cols, CV_8UC1);
    for (int i = 0; i < grayImg.rows - 1; i++){
        for (int j = 0; j < grayImg.cols - 1, j++){
            grad.at<uchar>(i, j) = saturate_cast<uchar>(fabs(grayImg.at<uchar>(i, j) - grayImg.at<uchar>(i+1, j+1)) +
                                                        fabs(grayImg.at<uchar>(i+1, j))- grayImg.at<uchar>(i, j+1));
            gray_img.at<uchar>(i, j) = saturate_cast<uchar>(grayImg.at<uchar>(i, j) - grad.at<uchar>(i, j));
        }
    }
    QTemp1 = QImage((const uchar*)(gray_img.data), gray_img.cols, gray_img.rows, gray_img.cols * gray_img.channels(), QImage::Format_Indexed8);
    ui->label_3->setPixmap(QPixmap::fromImage(QTemp1));
    QTemp1 = QTemp1.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_3->setScaledContents(true);
    ui->label_3->resize(QTemp1.size);
    ui->label_3->show();

    QTemp2 = QImage((const uchar*)(grad.data), grad.cols, grad.rows, grad.cols * grad.channels(), QImage::Format_Indexed8);
    Ui->label_2->setPixmap(QPixmap::fromImage(QTemp2));
    QTemp2 = QTemp2.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setScaledContents(true);
    ui->label_2->resize(QTemp2.size);
    ui->label_2->show();
}

void MainWindow::on_sobel_edge_detection_clicked(){
    Mat grad, gray_img, fx, fy;
    QImage QTemp1, QTemp2;
    grad.create(grayImg.rows, grayImg.cols, CV_8UC1);
    gray_img.create(grayImg.rows, grayImg.cols, CV_8UC1);
    fx.create(grayImg.rows, grayImg.cols, CV_8UC1);
    fy.create(grayImg.rows, grayImg.cols, CV_8UC1);
    for (int i = 1; i < grayImg.rows - 1; i++){
        for (int j = 1; j < grayImg.cols - 1, j++){
            fx.at<uchar>(i, j) = saturate_cast<uchar>(fabs(grayImg.at<uchar>(i+1, j-1) + 2 * grayImg.at<uchar>(i+1, j)
                                                    + grayImg.at<uchar>(i+1, j+1) - grayImg.at<uchar>(i-1, j-1)
                                                    - 2 * grayImg.at<uchar>(i-1, j) - grayImg.at<uchar>(i-1, j+1)));
            fy.at<uchar>(i, j) = saturate_cast<uchar>(fabs(grayImg.at<uchar>(i-1, j+1) + 2 * grayImg.at<uchar>(i, j+1)
                                                    + grayImg.at<uchar>(i+1, j+1) - grayImg.at<uchar>(i-1, j-1)
                                                    - 2 * grayImg.at<uchar>(i, j-1) - grayImg.at<uchar>(i+1, j-1)));
            grad.at<uchar>(i, j) = fx.at<uchar>(i, j) + fy.at<uchar>(i, j);
            gray_img.at<uchar>(i, j) = saturate_cast<uchar>(grayImg.at<uchar>(i, j) - grad.at<uchar>(i, j));
        }
    }
    QTemp1 = QImage((const uchar*)(gray_img.data), gray_img.cols, gray_img.rows, gray_img.cols * gray_img.channels(), QImage::Format_Indexed8);
    ui->label_3->setPixmap(QPixmap::fromImage(QTemp1));
    QTemp1 = QTemp1.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_3->setScaledContents(true);
    ui->label_3->resize(QTemp1.size);
    ui->label_3->show();

    QTemp2 = QImage((const uchar*)(grad.data), grad.cols, grad.rows, grad.cols * grad.channels(), QImage::Format_Indexed8);
    Ui->label_2->setPixmap(QPixmap::fromImage(QTemp2));
    QTemp2 = QTemp2.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setScaledContents(true);
    ui->label_2->resize(QTemp2.size);
    ui->label_2->show();

}

void MainWindow::on_prewitt_edge_detection_clicked(){
    Mat grad, gray_img, fx, fy;
    QImage QTemp1, QTemp2;
    grad.create(grayImg.rows, grayImg.cols, CV_8UC1);
    gray_img.create(grayImg.rows, grayImg.cols, CV_8UC1);
    fx.create(grayImg.rows, grayImg.cols, CV_8UC1);
    fy.create(grayImg.rows, grayImg.cols, CV_8UC1);
    for (int i = 1; i < grayImg.rows - 1; i++){
        for (int j = 1; j < grayImg.cols - 1, j++){
            fx.at<uchar>(i, j) = saturate_cast<uchar>(fabs(grayImg.at<uchar>(i+1, j-1) + grayImg.at<uchar>(i+1, j)
                                                           + grayImg.at<uchar>(i+1, j+1) - grayImg.at<uchar>(i-1, j-1)
                                                           - grayImg.at<uchar>(i-1, j) - grayImg.at<uchar>(i-1, j+1)));
            fy.at<uchar>(i, j) = saturate_cast<uchar>(fabs(grayImg.at<uchar>(i-1, j+1) + grayImg.at<uchar>(i, j+1)
                                                           + grayImg.at<uchar>(i+1, j+1) - grayImg.at<uchar>(i-1, j-1)
                                                           - grayImg.at<uchar>(i, j-1) - grayImg.at<uchar>(i+1, j-1)));
//            grad.at<uchar>(i, j) = fx.at<uchar>(i, j) + fy.at<uchar>(i, j);
            grad.at<uchar>(i, j) = max(fx.at<uchar>(i, j), fy.at<uchar>(i, j));
            gray_img.at<uchar>(i, j) = saturate_cast<uchar>(grayImg.at<uchar>(i, j) - grad.at<uchar>(i, j));
        }
    }
    QTemp1 = QImage((const uchar*)(gray_img.data), gray_img.cols, gray_img.rows, gray_img.cols * gray_img.channels(), QImage::Format_Indexed8);
    ui->label_3->setPixmap(QPixmap::fromImage(QTemp1));
    QTemp1 = QTemp1.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_3->setScaledContents(true);
    ui->label_3->resize(QTemp1.size);
    ui->label_3->show();

    QTemp2 = QImage((const uchar*)(grad.data), grad.cols, grad.rows, grad.cols * grad.channels(), QImage::Format_Indexed8);
    Ui->label_2->setPixmap(QPixmap::fromImage(QTemp2));
    QTemp2 = QTemp2.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setScaledContents(true);
    ui->label_2->resize(QTemp2.size);
    ui->label_2->show();

}

void MainWindow::on_laplacian_edge_detection_clicked(){
    Mat grad, gray_img
    QImage QTemp1, QTemp2;
    grad.create(grayImg.rows, grayImg.cols, CV_8UC1);
    gray_img.create(grayImg.rows, grayImg.cols, CV_8UC1);
    for (int i = 1; i < grayImg.rows - 1; i++){
        for (int j = 1; j < grayImg.cols - 1, j++){
            grad.at<uchar>(i, j) = saturate_cast<uchar>(grayImg.at<uchar>(i-1, j) - 4 * grayImg.at<uchar>(i, j)
                                                        + grayImg.at<uchar>(i+1, j) + grayImg.at<uchar>(i, j-1)
                                                        + grayImg.at<uchar>(i, j+1));
            gray_img.at<uchar>(i, j) = saturate_cast<uchar>(- grayImg.at<uchar>(i-1, j) + 5 * grayImg.at<uchar>(i, j)
                                                            - grayImg.at<uchar>(i+1, j) - grayImg.at<uchar>(i, j-1)
                                                            - grayImg.at<uchar>(i, j+1))
        }
    }
    QTemp1 = QImage((const uchar*)(gray_img.data), gray_img.cols, gray_img.rows, gray_img.cols * gray_img.channels(), QImage::Format_Indexed8);
    ui->label_3->setPixmap(QPixmap::fromImage(QTemp1));
    QTemp1 = QTemp1.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_3->setScaledContents(true);
    ui->label_3->resize(QTemp1.size);
    ui->label_3->show();

    QTemp2 = QImage((const uchar*)(grad.data), grad.cols, grad.rows, grad.cols * grad.channels(), QImage::Format_Indexed8);
    Ui->label_2->setPixmap(QPixmap::fromImage(QTemp2));
    QTemp2 = QTemp2.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setScaledContents(true);
    ui->label_2->resize(QTemp2.size);
    ui->label_2->show();
}

void MainWindow::on_krisch_edge_detection_clicked(){
    Mat grad, gray_img, f1, f2, f3, f4, f5, f6, f7, f8;
    QImage QTemp1, QTemp2;
    grad.create(grayImg.rows, grayImg.cols, CV_8UC1);
    gray_img.create(grayImg.rows, grayImg.cols, CV_8UC1);
    f1.create(grayImg.rows, grayImg.cols, CV_8UC1);
    f2.create(grayImg.rows, grayImg.cols, CV_8UC1);
    f3.create(grayImg.rows, grayImg.cols, CV_8UC1);
    f4.create(grayImg.rows, grayImg.cols, CV_8UC1);
    f5.create(grayImg.rows, grayImg.cols, CV_8UC1);
    f6.create(grayImg.rows, grayImg.cols, CV_8UC1);
    f7.create(grayImg.rows, grayImg.cols, CV_8UC1);
    f8.create(grayImg.rows, grayImg.cols, CV_8UC1);
    for (int i = 1; i < grayImg.rows - 1; i++){
        for (int j = 1; j < grayImg.cols - 1, j++){
            f1.at<uchar>(i, j) = saturate_cast<uchar>(fabs(- 5 * grayImg.at<uchar>(i-1, j-1) - 5 * grayImg.at<uchar>(i-1, j)
                                                            - 5 * grayImg.at<uchar>(i-1, j+1) + 3 * grayImg.at<uchar>(i, j-1)
                                                            + 3 * grayImg.at<uchar>(i, j+1) + 3 * grayImg.at<uchar>(i+1, j-1)
                                                            + 3 * grayImg.at<uchar>(i+1, j) + 3 * grayImg.at<uchar>(i+1, j+1)));
            f2.at<uchar>(i, j) = saturate_cast<uchar>(fabs( 3 * grayImg.at<uchar>(i-1, j-1) - 5 * grayImg.at<uchar>(i-1, j)
                                                           - 5 * grayImg.at<uchar>(i-1, j+1) + 3 * grayImg.at<uchar>(i, j-1)
                                                           - 5 * grayImg.at<uchar>(i, j+1) + 3 * grayImg.at<uchar>(i+1, j-1)
                                                           + 3 * grayImg.at<uchar>(i+1, j) + 3 * grayImg.at<uchar>(i+1, j+1)));
            f3.at<uchar>(i, j) = saturate_cast<uchar>(fabs( 3 * grayImg.at<uchar>(i-1, j-1) + 3 * grayImg.at<uchar>(i-1, j)
                                                           - 5 * grayImg.at<uchar>(i-1, j+1) + 3 * grayImg.at<uchar>(i, j-1)
                                                           - 5 * grayImg.at<uchar>(i, j+1) + 3 * grayImg.at<uchar>(i+1, j-1)
                                                           + 3 * grayImg.at<uchar>(i+1, j) - 5 * grayImg.at<uchar>(i+1, j+1)));
            f4.at<uchar>(i, j) = saturate_cast<uchar>(fabs( 3 * grayImg.at<uchar>(i-1, j-1) + 3 * grayImg.at<uchar>(i-1, j)
                                                           + 3 * grayImg.at<uchar>(i-1, j+1) + 3 * grayImg.at<uchar>(i, j-1)
                                                           - 5 * grayImg.at<uchar>(i, j+1) + 3 * grayImg.at<uchar>(i+1, j-1)
                                                           - 5 * grayImg.at<uchar>(i+1, j) - 5 * grayImg.at<uchar>(i+1, j+1)));
            f5.at<uchar>(i, j) = saturate_cast<uchar>(fabs( 3 * grayImg.at<uchar>(i-1, j-1) + 3 * grayImg.at<uchar>(i-1, j)
                                                           + 3 * grayImg.at<uchar>(i-1, j+1) + 3 * grayImg.at<uchar>(i, j-1)
                                                           + 3 * grayImg.at<uchar>(i, j+1) - 5 * grayImg.at<uchar>(i+1, j-1)
                                                           - 5 * grayImg.at<uchar>(i+1, j) - 5 * grayImg.at<uchar>(i+1, j+1)));
            f6.at<uchar>(i, j) = saturate_cast<uchar>(fabs( 3 * grayImg.at<uchar>(i-1, j-1) + 3 * grayImg.at<uchar>(i-1, j)
                                                            + 3 * grayImg.at<uchar>(i-1, j+1) - 5 * grayImg.at<uchar>(i, j-1)
                                                            + 3 * grayImg.at<uchar>(i, j+1) - 5 * grayImg.at<uchar>(i+1, j-1)
                                                            - 5 * grayImg.at<uchar>(i+1, j) + 3 * grayImg.at<uchar>(i+1, j+1)));
            f7.at<uchar>(i, j) = saturate_cast<uchar>(fabs( - 5 * grayImg.at<uchar>(i-1, j-1) + 3 * grayImg.at<uchar>(i-1, j)
                                                            + 3 * grayImg.at<uchar>(i-1, j+1) - 5 * grayImg.at<uchar>(i, j-1)
                                                            + 3 * grayImg.at<uchar>(i, j+1) - 5 * grayImg.at<uchar>(i+1, j-1)
                                                            + 3 * grayImg.at<uchar>(i+1, j) + 3 * grayImg.at<uchar>(i+1, j+1)));
            f8.at<uchar>(i, j) = saturate_cast<uchar>(fabs( - 5 * grayImg.at<uchar>(i-1, j-1) - 5 * grayImg.at<uchar>(i-1, j)
                                                            + 3 * grayImg.at<uchar>(i-1, j+1) - 5 * grayImg.at<uchar>(i, j-1)
                                                            + 3 * grayImg.at<uchar>(i, j+1) + 3 * grayImg.at<uchar>(i+1, j-1)
                                                            + 3 * grayImg.at<uchar>(i+1, j) + 3 * grayImg.at<uchar>(i+1, j+1)));
            QVector<int> f = {f1.at<uchar>(i, j), f2.at<uchar>(i, j), f3.at<uchar>(i, j), f4.at<uchar>(i, j),
                              f5.at<uchar>(i, j), f6.at<uchar>(i, j), f7.at<uchar>(i, j), f8.at<uchar>(i, j)};
            auto max_f = std::max_element(std::begin(f), std::end(f));
            grad.at<uchar>(i, j) = max_f;
            gray_img.at<uchar>(i, j) = saturate_cast<uchar>(grayImg.at<uchar>(i, j) - grad.at<uchar>(i, j));
        }
    }
    QTemp1 = QImage((const uchar*)(gray_img.data), gray_img.cols, gray_img.rows, gray_img.cols * gray_img.channels(), QImage::Format_Indexed8);
    ui->label_3->setPixmap(QPixmap::fromImage(QTemp1));
    QTemp1 = QTemp1.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_3->setScaledContents(true);
    ui->label_3->resize(QTemp1.size);
    ui->label_3->show();

    QTemp2 = QImage((const uchar*)(grad.data), grad.cols, grad.rows, grad.cols * grad.channels(), QImage::Format_Indexed8);
    Ui->label_2->setPixmap(QPixmap::fromImage(QTemp2));
    QTemp2 = QTemp2.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setScaledContents(true);
    ui->label_2->resize(QTemp2.size);
    ui->label_2->show();

}

void MainWindow::on_canny_edge_detection_clicked(){
    Mat grad, gray_img, gaussian_signal, fx, fy, max_control;
    grad.create(grayImg.rows, grayImg.cols, CV_8UC1);
    gray_img.create(grayImg.rows, grayImg.cols, CV_8UC1);
    guassian_signal.create(grayImg.rows, grayImg.cols, CV_8UC1);
    fx.create(grayImg.rows, grayImg.cols, CV_8UC1);
    fy.create(grayImg.rows, grayImg.cols, CV_8UC1);
    QVector<double> direction((grayImg.rows-1) * (grayImg.cols-1), 0);

    for (int i = 0; i < grayImg.rows - 1, i++){
        for (int j = 0; j < grayImg.cols - 1, j ++){
            gaussian_signal.at<uchar>(i, j) = saturate_cast<uchar>(fabs(( 1 * grayImg.at<uchar>(i-1, j-1) + 2 * grayImg.at<uchar>(i-1, j)
                                                                              + 1 * grayImg.at<uchar>(i-1, j+1) + 2 * grayImg.at<uchar>(i, j-1)
                                                                              + 4 * grayImg.at<uchar>(i, j) + 2 * grayImg.at<uchar>(i, j+1)
                                                                              + 1 * grayImg.at<uchar>(i+1, j-1) + 2 * grayImg.at<uchar>(i+1, j)
                                                                              + 2 * grayImg.at<uchar>(i+1, j+1)) / 16 ));
        }
    }
    int k = 0;
    for (int i = 1; i < guassian_signal.rows - 1; i++){
        for (int j = 1; j < guassian_signal.cols - 1; j++){
            fx.at<uchar>(i, j) = saturate_cast<uchar>(fabs( - grayImg.at<uchar>(i-1, j-1) - 2 * grayImg.at<uchar>(i-1, j)
                                                        - grayImg.at<uchar>(i-1, j+1) + grayImg.at<uchar>(i+1, j-1)
                                                        + 2 * grayImg.at<uchar>(i+1, j) + grayImg.at<uchar>(i+1, j+1)));
            fy.at<uchar>(i, j) = saturate_cast<uchar>(fabs( - grayImg.at<uchar>(i-1, j-1) - 2 * grayImg.at<uchar>(i, j-1)
                                                            - grayImg.at<uchar>(i+1, j-1) + grayImg.at<uchar>(i-1, j+1)
                                                            + 2 * grayImg.at<uchar>(i, j+1) + grayImg.at<uchar>(i+1, j+1)));
            grad.at<uchar>(i, j) = sqrt(pow(fx.at<uchar>(i, j), 2) + pow(fy.at<uchar>(i, j), 2));

            if (fx.at<uchar>(i, j) == 0){
                fx.at<uchar>(i, j) == 1e-6
            }
            directioin[k] = atan(fy.at<uchar>(i, j) / fx.at<uchar>(i, j)) * 57.3;
            direction[k] += 90;
            k++;

        }
    }
    max_control = grad.clone();
    k = 0;
    for (int i = 1; i < grad.rows - 1; i++){
        for (int j = 1; j < grad.cols - 1; j++){
            if (direction[k] > 0 && direction[k] <= 45){
                if (grad.at<uchar>(i, j) <= (grad.at<uchar>(i, j+1) + (grad.at<uchar>(i-1, j+1) - grad.at<uchar>(i, j+1))
                        * tan(direction[i * max_control.rows + j])) || (grad.at<uchar>(i, j) <= (grad.at<uchar>(i, j-1)
                        + (grad.at<uchar>(i+1, j-1) - grad.at<uchar>(i, j-1)) * tan(direction[i * max_control.rows + j])))){
                    max_control.at<uchar>(i, j) = 0
                }
            }

            if (direction[k] > 45 && direction[k] <= 90){
                if (grad.at<uchar>(i, j) <= (grad.at<uchar>(i-1, j) + (grad.at<uchar>(i-1, j+1) - grad.at<uchar>(i-1, j+1)
                * tan(direction[i * max_control.rows + j])) || (grad.at<uchar>(i, j) <= (grad.at<uchar>(i+1, j)
                + (grad.at<uchar>(i+1, j-1) - grad.at<uchar>(i+1, j)) * tan(direction[i * max_control.rows + j])))){
                    max_control.at<uchar>(i, j) = 0
                }
            }

            if (direction[k] > 90 && direction[k] <= 135){
                if (grad.at<uchar>(i, j) <= (grad.at<uchar>(i-1, j) + (grad.at<uchar>(i-1, j-1) - grad.at<uchar>(i-1, j)
                * tan(direction[i * max_control.rows + j])) || (grad.at<uchar>(i, j) <= (grad.at<uchar>(i+1, j)
                + (grad.at<uchar>(i+1, j+1) - grad.at<uchar>(i+1, j)) * tan(direction[i * max_control.rows + j])))){
                    max_control.at<uchar>(i, j) = 0
                }
            }

            if (direction[k] > 135 && direction[k] <= 180){
                if (grad.at<uchar>(i, j) <= (grad.at<uchar>(i, j-1) + (grad.at<uchar>(i-1, j-1) - grad.at<uchar>(i, j-1)
                * tan(direction[i * max_control.rows + j])) || (grad.at<uchar>(i, j) <= (grad.at<uchar>(i, j+1)
                + (grad.at<uchar>(i+1, j+1) - grad.at<uchar>(i, j)) * tan(direction[i * max_control.rows + j])))){
                    max_control.at<uchar>(i, j) = 0
                }
            }
            k++;
        }
    }
    double_threshold(max_control, 15, 55);
    double_threshold_link(max_control, 15, 55);
    for (int i = 0; i < grayImg.rows - 1; i++){
        for (int j = 0; j < grayImg.cols - 1; j++){
            gray_img.at<uchar>(i, j) = saturate_cast<uchar>(grayImg.at<uchar>(i, j) - max_control.at<uchar>(i, j));
        }
    }

    QTemp1 = QImage((const uchar*)(gray_img.data), gray_img.cols, gray_img.rows, gray_img.cols * gray_img.channels(), QImage::Format_Indexed8);
    ui->label_3->setPixmap(QPixmap::fromImage(QTemp1));
    QTemp1 = QTemp1.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_3->setScaledContents(true);
    ui->label_3->resize(QTemp1.size);
    ui->label_3->show();

    QTemp2 = QImage((const uchar*)(max_control.data), max_control.cols, max_control.rows, max_control.cols * max_control.channels(), QImage::Format_Indexed8);
    Ui->label_2->setPixmap(QPixmap::fromImage(QTemp2));
    QTemp2 = QTemp2.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setScaledContents(true);
    ui->label_2->resize(QTemp2.size);
    ui->label_2->show();

}

void MainWindow::on_window_filter_clicked(){
    Mat filter_img;
    QImage QTemp;
    QVector<double> window(8, 0), min_img(8, 0);

    filter_img = guassianNoiseImg.clone();

    for (int i = 1; i < guassianNoiseImg.rows - 1; i++){
        for (int j = 1; j < guassianNoiseImg.cols - 1; j++){
            for (int k = 0; k < 3; k++){
                window[0] = (guassianNoiseImg.at<Vec3b>(i-1, j-1)[k] + guassianNoiseImg.at<Vec3b>(i-1, j)[k] +
                        guassianNoiseImg.at<Vec3b>(i, j-1)[k] + guassianNoiseImg.at<Vec3b>(i, j)[k]) / 4;
                window[1] = (guassianNoiseImg.at<Vec3b>(i-1, j)[k] + guassianNoiseImg.at<Vec3b>(i-1, j+1)[k] +
                             guassianNoiseImg.at<Vec3b>(i, j)[k] + guassianNoiseImg.at<Vec3b>(i, j+1)[k]) / 4;
                window[2] = (guassianNoiseImg.at<Vec3b>(i, j)[k] + guassianNoiseImg.at<Vec3b>(i, j+1)[k] +
                             guassianNoiseImg.at<Vec3b>(i+1, j)[k] + guassianNoiseImg.at<Vec3b>(i+1, j+1)[k]) / 4;
                window[3] = (guassianNoiseImg.at<Vec3b>(i, j-1)[k] + guassianNoiseImg.at<Vec3b>(i, j)[k] +
                             guassianNoiseImg.at<Vec3b>(i+1, j-1)[k] + guassianNoiseImg.at<Vec3b>(i+1, j)[k]) / 4;
                window[4] = (guassianNoiseImg.at<Vec3b>(i-1, j-1)[k] + guassianNoiseImg.at<Vec3b>(i-1, j)[k] +
                             guassianNoiseImg.at<Vec3b>(i-1, j+1)[k] + guassianNoiseImg.at<Vec3b>(i, j-1)[k] +
                             guassianNoiseImg.at<Vec3b>(i, j)[k] + guassianNoiseImg.at<Vec3b>(i, j+1)[k]) / 6;
                window[5] = (guassianNoiseImg.at<Vec3b>(i, j-1)[k] + guassianNoiseImg.at<Vec3b>(i, j)[k] +
                             guassianNoiseImg.at<Vec3b>(i, j+1)[k] + guassianNoiseImg.at<Vec3b>(i+1, j-1)[k] +
                             guassianNoiseImg.at<Vec3b>(i+1, j)[k] + guassianNoiseImg.at<Vec3b>(i+1, j+1)[k]) / 6;
                window[6] = (guassianNoiseImg.at<Vec3b>(i-1, j-1)[k] + guassianNoiseImg.at<Vec3b>(i, j-1)[k] +
                             guassianNoiseImg.at<Vec3b>(i+1, j-1)[k] + guassianNoiseImg.at<Vec3b>(i-1, j)[k] +
                             guassianNoiseImg.at<Vec3b>(i, j)[k] + guassianNoiseImg.at<Vec3b>(i+1, j)[k]) / 6;
                window[7] = (guassianNoiseImg.at<Vec3b>(i-1, j)[k] + guassianNoiseImg.at<Vec3b>(i, j)[k] +
                             guassianNoiseImg.at<Vec3b>(i+1, j)[k] + guassianNoiseImg.at<Vec3b>(i-1, j+1)[k] +
                             guassianNoiseImg.at<Vec3b>(i, j+1)[k] + guassianNoiseImg.at<Vec3b>(i+1, j+1)[k]) / 6;

                for (int n = 0; n < 8; n++){
                    min_img[n] = pow(window[n] - guassianNoiseImg.at<Vec3b>(i, j)[k], 2);
                }
                auto smallest = std::min_element(std::begin(min_img), std::end(min_img));
                int position = std::distance(std::begin(min_img), smallest);
                filter_img.at<Vec3b>(i, j)[k] = saturate_cast<uchar>(window[position]);

            }
        }
    }

    QTemp = QImage((const uchar*)(filter_img.data), filter_img.cols, filter_img.rows, filter_img.cols * filter_img.channels(), QImage::Format_Indexed8);
    ui->label_3->setPixmap(QPixmap::fromImage(QTemp));
    QTemp = QTemp.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_3->setScaledContents(true);
    ui->label_3->resize(QTemp.size);
    ui->label_3->show();
}

void MainWindow::on_average_filter_clicked(){
    Mat filter_img;
    QImage QTemp;

    filter_img = guassianNoiseImg.clone();

    for (int i = 1; i < guassianNoiseImg.rows - 1; i++){
        for (int j = 1; j < guassianNoiseImg.cols - 1; j++){
            for (int k = 0; k < 3; k++){
                filter_img.at<Vec3b>(i, j)[k] = saturate_cast<uchar>((guassianNoiseImg.at<Vec3b>(i-1, j-1)[k] +
                        guassianNoiseImg.at<Vec3b>(i-1, j)[k] + guassianNoiseImg.at<Vec3b>(i-1, j+1)[k] +
                        guassianNoiseImg.at<Vec3b>(i, j-1)[k] + guassianNoiseImg.at<Vec3b>(i, j)[k] +
                        guassianNoiseImg.at<Vec3b>(i, j+1)[k] + guassianNoiseImg.at<Vec3b>(i+1, j-1)[k] +
                        guassianNoiseImg.at<Vec3b>(i+1, j)[k] + guassianNoiseImg.at<Vec3b>(i+1, j+1)[k]) / 9);
            }
        }
    }

    QTemp = QImage((const uchar*)(filter_img.data), filter_img.cols, filter_img.rows, filter_img.cols * filter_img.channels(), QImage::Format_Indexed8);
    ui->label_3->setPixmap(QPixmap::fromImage(QTemp));
    QTemp = QTemp.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_3->setScaledContents(true);
    ui->label_3->resize(QTemp.size);
    ui->label_3->show();

}

void MainWindow::on_middle_filter_clicked(){
    Mat filter_img;
    QImage QTemp;
    QVector<double> middle(9, 0);

    filter_img = guassianNoiseImg.clone();

    for (int i = 1; i < guassianNoiseImg.rows - 1; i++){
        for (int j = 1; j < guassianNoiseImg.cols - 1; j++){
            for (int k = 0; k < 3; k++){
                middle = [guassianNoiseImg.at<Vec3b>(i-1, j-1)[k], guassianNoiseImg.at<Vec3b>(i-1, j)[k],
                        guassianNoiseImg.at<Vec3b>(i-1, j+1)[k], guassianNoiseImg.at<Vec3b>(i, j-1)[k],
                        guassianNoiseImg.at<Vec3b>(i, j)[k], guassianNoiseImg.at<Vec3b>(i, j+1)[k],
                        guassianNoiseImg.at<Vec3b>(i+1, j-1)[k], guassianNoiseImg.at<Vec3b>(i+1, j)[k],
                        guassianNoiseImg.at<Vec3b>(i+1, j+1)[k]];
                std::sort(std::begin(middle), std::end(middle))
                filter_img.at<Vec3b>(i, j)[k] = saturate_cast<uchar>(middle[5]);
            }
        }
    }

    QTemp = QImage((const uchar*)(filter_img.data), filter_img.cols, filter_img.rows, filter_img.cols * filter_img.channels(), QImage::Format_Indexed8);
    ui->label_3->setPixmap(QPixmap::fromImage(QTemp));
    QTemp = QTemp.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_3->setScaledContents(true);
    ui->label_3->resize(QTemp.size);
    ui->label_3->show();
}

void MainWindow::on_gaussian_filter_clicked(){
    Mat filter_img;
    QImage QTemp;
    QVector<double> middle(9, 0);

    filter_img = guassianNoiseImg.clone();

    for (int i = 1; i < guassianNoiseImg.rows - 1; i++){
        for (int j = 1; j < guassianNoiseImg.cols - 1; j++){
            for (int k = 0; k < 3; k++){
                filter_img.at<Vec3b>(i, j)[k] = saturate_cast<uchar>((guassianNoiseImg.at<Vec3b>(i-1, j-1)[k] +
                        2 * guassianNoiseImg.at<Vec3b>(i-1, j)[k] + guassianNoiseImg.at<Vec3b>(i-1, j+1)[k] +
                        2 * guassianNoiseImg.at<Vec3b>(i, j-1)[k] + 4 * guassianNoiseImg.at<Vec3b>(i, j)[k] +
                        2 * guassianNoiseImg.at<Vec3b>(i, j+1)[k] + guassianNoiseImg.at<Vec3b>(i+1, j-1)[k] +
                        2 * guassianNoiseImg.at<Vec3b>(i+1, j)[k] + guassianNoiseImg.at<Vec3b>(i+1, j+1)[k]) / 16);
            }
        }
    }

    QTemp = QImage((const uchar*)(filter_img.data), filter_img.cols, filter_img.rows, filter_img.cols * filter_img.channels(), QImage::Format_Indexed8);
    ui->label_3->setPixmap(QPixmap::fromImage(QTemp));
    QTemp = QTemp.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_3->setScaledContents(true);
    ui->label_3->resize(QTemp.size);
    ui->label_3->show();
}

void MainWindow::on_form_filter_clicked(){
    Mat filter_img, rgb_img, temp;
    QImage QTemp;

    Mat element = getStructuringElement(MORPH_RECT,Size(15,15));
    cvtColor(srcImg, rgb_img, CV_BGR2RGB);
    erode(rgb_img, temp, element);
    dilate(temp, filter_img, element);

    QTemp = QImage((const uchar*)(filter_img.data), filter_img.cols, filter_img.rows, filter_img.step, QImage::Format_RGB888);
    ui->label_3->setPixmap(QPixmap::fromImage(QTemp));
    QTemp = QTemp.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_3->setScaledContents(true);
    ui->label_3->resize(QTemp.size);
    ui->label_3->show();
}