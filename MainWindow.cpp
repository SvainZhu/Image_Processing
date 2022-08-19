//
// Created by SvainZhu on 2022/5/19.
//
#include <MainWindow.h>
#include <ui_mainwindow.h>

// init main window
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);
}

//deploy the class of MainWindow
MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::on_select_images_clicked(){
    QString img_name = QFileDialog::getOpenFileName(this, tr(""), "./resources/img/", "files(*)");
    srcImg = imread(img_name.toStdString());

    Mat temp;
    QImage QTemp;
    cvtColor(srcImg, temp, CV_BGR2RGB);
    QTemp = QImage((const unsigned char *)(temp.data), temp.cols, temp.rows, temp.step, QImage::Format_RGB888);

    ui->label->setPixmap(QPixmap::fromImage(QTemp));
    QTemp = QTemp.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label->setScaledContents(true);
    ui->label->resize(QTemp.size());
    ui->label->show();

}

void MainWindow::on_rgb_to_gray_clicked(){
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
    ui->label_1->resize(QTemp.size());
    ui->label_1->show();

}

void MainWindow::on_gray_hist_clicked(){
    QImage QTemp;
    QTemp = QImage((const uchar*)(grayImg.data), grayImg.cols, grayImg.rows, grayImg.cols * grayImg.channels(), QImage::Format_Indexed8);
    ui->label_2->setPixmap(QPixmap::fromImage(QTemp));
    QTemp = QTemp.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setScaledContents(true);
    ui->label_2->resize(QTemp.size());
    ui->label_2->show();

    Mat gray_hist;
    gray_hist = gray_to_hist(grayImg);
    imshow("gray histogram", gray_hist);
    waitKey(0);
    cv::destroyWindow("gray histogram");
    waitKey(1);
}

void MainWindow::on_gray_balance_clicked(){
    Mat balance, gray_img;
    gray_img.create(srcImg.rows, srcImg.cols, CV_8UC1);
    balance.create(srcImg.rows, srcImg.cols, CV_8UC1);

    QImage QTemp;
    QVector<int> pixel(256, 0);
    QVector<float> pixel_gray(256.,0.);
    float sum = 0;

    for (int i = 0; i < grayImg.rows; i++){
        for (int j = 0; j < grayImg.cols; j++){
            pixel[grayImg.at<uchar>(i, j)]++;
        }
    }

    for (int i = 0; i < pixel.size(); i++){
        sum += pixel[i];
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
    ui->label_3->resize(QTemp.size());
    ui->label_3->show();

}

void MainWindow::on_grad_sharpen_clicked(){
    Mat grad, gray_img;
    QImage QTemp1, QTemp2;
    grad.create(srcImg.rows, srcImg.cols, CV_8UC1);
    gray_img.create(srcImg.rows, srcImg.cols, CV_8UC1);
    for (int i = 0; i < gray_img.rows - 1; i++){
        for (int j = 0; j < gray_img.cols - 1; j++){
            grad.at<uchar>(i, j) = saturate_cast<uchar>(max(fabs(grayImg.at<uchar>(i+1, j) - grayImg.at<uchar>(i, j)),
                                                            fabs(grayImg.at<uchar>(i, j+1) - grayImg.at<uchar>(i, j))));
            gray_img.at<uchar>(i, j) = saturate_cast<uchar>(grayImg.at<uchar>(i, j) - grad.at<uchar>(i, j));
        }
    }

    QTemp1 = QImage((const uchar*)(gray_img.data), gray_img.cols, gray_img.rows, gray_img.cols * gray_img.channels(), QImage::Format_Indexed8);
    ui->label_3->setPixmap(QPixmap::fromImage(QTemp1));
    QTemp1 = QTemp1.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_3->setScaledContents(true);
    ui->label_3->resize(QTemp1.size());
    ui->label_3->show();

    QTemp2 = QImage((const uchar*)(grad.data), grad.cols, grad.rows, grad.cols * grad.channels(), QImage::Format_Indexed8);
    ui->label_2->setPixmap(QPixmap::fromImage(QTemp2));
    QTemp2 = QTemp2.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setScaledContents(true);
    ui->label_2->resize(QTemp2.size());
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
                                                            - grayImg.at<uchar>(i, j+1));
        }
    }

    QTemp1 = QImage((const uchar*)(gray_img.data), gray_img.cols, gray_img.rows, gray_img.cols * gray_img.channels(), QImage::Format_Indexed8);
    ui->label_3->setPixmap(QPixmap::fromImage(QTemp1));
    QTemp1 = QTemp1.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_3->setScaledContents(true);
    ui->label_3->resize(QTemp1.size());
    ui->label_3->show();

    QTemp2 = QImage((const uchar*)(grad.data), grad.cols, grad.rows, grad.cols * grad.channels(), QImage::Format_Indexed8);
    ui->label_2->setPixmap(QPixmap::fromImage(QTemp2));
    QTemp2 = QTemp2.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setScaledContents(true);
    ui->label_2->resize(QTemp2.size());
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

void MainWindow::on_add_gaussian_noise_clicked(){
    Mat gaussian_noise, temp;
    QImage QTemp;
    gaussian_noise.create(srcImg.rows, srcImg.cols, CV_8UC3);
    gaussian_noise = add_gaussian_noise(srcImg, 500);
    cvtColor(gaussian_noise, temp, CV_BGR2RGB);

    guassianNoiseImg = temp.clone();
    QTemp = QImage((const uchar*)(temp.data), temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
    ui->label_3->setPixmap(QPixmap::fromImage(QTemp));
    QTemp = QTemp.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_3->setScaledContents(true);
    ui->label_3->resize(QTemp.size());
    ui->label_3->show();

}

void MainWindow::on_roberts_edge_detection_clicked(){
    Mat grad, gray_img;
    QImage QTemp1, QTemp2;
    grad.create(grayImg.rows, grayImg.cols, CV_8UC1);
    gray_img.create(grayImg.rows, grayImg.cols, CV_8UC1);
    for (int i = 0; i < grayImg.rows - 1; i++){
        for (int j = 0; j < grayImg.cols - 1; j++){
            grad.at<uchar>(i, j) = saturate_cast<uchar>(fabs(grayImg.at<uchar>(i, j) - grayImg.at<uchar>(i+1, j+1)) +
                                                        fabs(grayImg.at<uchar>(i+1, j))- grayImg.at<uchar>(i, j+1));
            gray_img.at<uchar>(i, j) = saturate_cast<uchar>(grayImg.at<uchar>(i, j) - grad.at<uchar>(i, j));
        }
    }
    QTemp1 = QImage((const uchar*)(gray_img.data), gray_img.cols, gray_img.rows, gray_img.cols * gray_img.channels(), QImage::Format_Indexed8);
    ui->label_3->setPixmap(QPixmap::fromImage(QTemp1));
    QTemp1 = QTemp1.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_3->setScaledContents(true);
    ui->label_3->resize(QTemp1.size());
    ui->label_3->show();

    QTemp2 = QImage((const uchar*)(grad.data), grad.cols, grad.rows, grad.cols * grad.channels(), QImage::Format_Indexed8);
    ui->label_2->setPixmap(QPixmap::fromImage(QTemp2));
    QTemp2 = QTemp2.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setScaledContents(true);
    ui->label_2->resize(QTemp2.size());
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
        for (int j = 1; j < grayImg.cols - 1; j++){
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
    ui->label_3->resize(QTemp1.size());
    ui->label_3->show();

    QTemp2 = QImage((const uchar*)(grad.data), grad.cols, grad.rows, grad.cols * grad.channels(), QImage::Format_Indexed8);
    ui->label_2->setPixmap(QPixmap::fromImage(QTemp2));
    QTemp2 = QTemp2.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setScaledContents(true);
    ui->label_2->resize(QTemp2.size());
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
        for (int j = 1; j < grayImg.cols - 1; j++){
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
    ui->label_3->resize(QTemp1.size());
    ui->label_3->show();

    QTemp2 = QImage((const uchar*)(grad.data), grad.cols, grad.rows, grad.cols * grad.channels(), QImage::Format_Indexed8);
    ui->label_2->setPixmap(QPixmap::fromImage(QTemp2));
    QTemp2 = QTemp2.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setScaledContents(true);
    ui->label_2->resize(QTemp2.size());
    ui->label_2->show();

}

void MainWindow::on_laplacian_edge_detection_clicked(){
    Mat grad, gray_img;
    QImage QTemp1, QTemp2;
    grad.create(grayImg.rows, grayImg.cols, CV_8UC1);
    gray_img.create(grayImg.rows, grayImg.cols, CV_8UC1);
    for (int i = 1; i < grayImg.rows - 1; i++){
        for (int j = 1; j < grayImg.cols - 1; j++){
            grad.at<uchar>(i, j) = saturate_cast<uchar>(grayImg.at<uchar>(i-1, j) - 4 * grayImg.at<uchar>(i, j)
                                                        + grayImg.at<uchar>(i+1, j) + grayImg.at<uchar>(i, j-1)
                                                        + grayImg.at<uchar>(i, j+1));
            gray_img.at<uchar>(i, j) = saturate_cast<uchar>(- grayImg.at<uchar>(i-1, j) + 5 * grayImg.at<uchar>(i, j)
                                                            - grayImg.at<uchar>(i+1, j) - grayImg.at<uchar>(i, j-1)
                                                            - grayImg.at<uchar>(i, j+1));
        }
    }
    QTemp1 = QImage((const uchar*)(gray_img.data), gray_img.cols, gray_img.rows, gray_img.cols * gray_img.channels(), QImage::Format_Indexed8);
    ui->label_3->setPixmap(QPixmap::fromImage(QTemp1));
    QTemp1 = QTemp1.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_3->setScaledContents(true);
    ui->label_3->resize(QTemp1.size());
    ui->label_3->show();

    QTemp2 = QImage((const uchar*)(grad.data), grad.cols, grad.rows, grad.cols * grad.channels(), QImage::Format_Indexed8);
    ui->label_2->setPixmap(QPixmap::fromImage(QTemp2));
    QTemp2 = QTemp2.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setScaledContents(true);
    ui->label_2->resize(QTemp2.size());
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
        for (int j = 1; j < grayImg.cols - 1; j++){
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
            int max_f = 0;
            for (int i = 0; i < 8; i++){
                if (f[i] > max_f){
                    max_f = f[i];
                }
            }
            grad.at<uchar>(i, j) = max_f;
            gray_img.at<uchar>(i, j) = saturate_cast<uchar>(grayImg.at<uchar>(i, j) - grad.at<uchar>(i, j));
        }
    }
    QTemp1 = QImage((const uchar*)(gray_img.data), gray_img.cols, gray_img.rows, gray_img.cols * gray_img.channels(), QImage::Format_Indexed8);
    ui->label_3->setPixmap(QPixmap::fromImage(QTemp1));
    QTemp1 = QTemp1.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_3->setScaledContents(true);
    ui->label_3->resize(QTemp1.size());
    ui->label_3->show();

    QTemp2 = QImage((const uchar*)(grad.data), grad.cols, grad.rows, grad.cols * grad.channels(), QImage::Format_Indexed8);
    ui->label_2->setPixmap(QPixmap::fromImage(QTemp2));
    QTemp2 = QTemp2.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setScaledContents(true);
    ui->label_2->resize(QTemp2.size());
    ui->label_2->show();

}

void MainWindow::on_canny_edge_detection_clicked(){
    Mat grad, gray_img, gaussian_signal, fx, fy, max_control;
    QImage QTemp1, QTemp2;
    grad.create(grayImg.rows, grayImg.cols, CV_8UC1);
    gray_img.create(grayImg.rows, grayImg.cols, CV_8UC1);
    gaussian_signal.create(grayImg.rows, grayImg.cols, CV_8UC1);
    fx.create(grayImg.rows, grayImg.cols, CV_8UC1);
    fy.create(grayImg.rows, grayImg.cols, CV_8UC1);
    QVector<double> direction((grayImg.rows-1) * (grayImg.cols-1), 0);

    for (int i = 0; i < grayImg.rows - 1; i++){
        for (int j = 0; j < grayImg.cols - 1; j ++){
            gaussian_signal.at<uchar>(i, j) = saturate_cast<uchar>(fabs(( 1 * grayImg.at<uchar>(i-1, j-1) + 2 * grayImg.at<uchar>(i-1, j)
                                                                              + 1 * grayImg.at<uchar>(i-1, j+1) + 2 * grayImg.at<uchar>(i, j-1)
                                                                              + 4 * grayImg.at<uchar>(i, j) + 2 * grayImg.at<uchar>(i, j+1)
                                                                              + 1 * grayImg.at<uchar>(i+1, j-1) + 2 * grayImg.at<uchar>(i+1, j)
                                                                              + 2 * grayImg.at<uchar>(i+1, j+1)) / 16 ));
        }
    }
    int k = 0;
    for (int i = 1; i < gaussian_signal.rows - 1; i++){
        for (int j = 1; j < gaussian_signal.cols - 1; j++){
            fx.at<uchar>(i, j) = saturate_cast<uchar>(fabs( - grayImg.at<uchar>(i-1, j-1) - 2 * grayImg.at<uchar>(i-1, j)
                                                        - grayImg.at<uchar>(i-1, j+1) + grayImg.at<uchar>(i+1, j-1)
                                                        + 2 * grayImg.at<uchar>(i+1, j) + grayImg.at<uchar>(i+1, j+1)));
            fy.at<uchar>(i, j) = saturate_cast<uchar>(fabs( - grayImg.at<uchar>(i-1, j-1) - 2 * grayImg.at<uchar>(i, j-1)
                                                            - grayImg.at<uchar>(i+1, j-1) + grayImg.at<uchar>(i-1, j+1)
                                                            + 2 * grayImg.at<uchar>(i, j+1) + grayImg.at<uchar>(i+1, j+1)));
            grad.at<uchar>(i, j) = sqrt(pow(fx.at<uchar>(i, j), 2) + pow(fy.at<uchar>(i, j), 2));

            if (fx.at<uchar>(i, j) == 0){
                fx.at<uchar>(i, j) = 1e-6;
            }
            direction[k] = atan(fy.at<uchar>(i, j) / fx.at<uchar>(i, j)) * 57.3;
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
                    max_control.at<uchar>(i, j) = 0;
                }
            }

            if (direction[k] > 45 && direction[k] <= 90){
                if (grad.at<uchar>(i, j) <= (grad.at<uchar>(i-1, j) + (grad.at<uchar>(i-1, j+1) - grad.at<uchar>(i-1, j+1))
                * tan(direction[i * max_control.rows + j])) || (grad.at<uchar>(i, j) <= (grad.at<uchar>(i+1, j)
                + (grad.at<uchar>(i+1, j-1) - grad.at<uchar>(i+1, j)) * tan(direction[i * max_control.rows + j])))){
                    max_control.at<uchar>(i, j) = 0;
                }
            }

            if (direction[k] > 90 && direction[k] <= 135){
                if (grad.at<uchar>(i, j) <= (grad.at<uchar>(i-1, j) + (grad.at<uchar>(i-1, j-1) - grad.at<uchar>(i-1, j))
                * tan(direction[i * max_control.rows + j])) || (grad.at<uchar>(i, j) <= (grad.at<uchar>(i+1, j)
                + (grad.at<uchar>(i+1, j+1) - grad.at<uchar>(i+1, j)) * tan(direction[i * max_control.rows + j])))){
                    max_control.at<uchar>(i, j) = 0;
                }
            }

            if (direction[k] > 135 && direction[k] <= 180){
                if (grad.at<uchar>(i, j) <= (grad.at<uchar>(i, j-1) + (grad.at<uchar>(i-1, j-1) - grad.at<uchar>(i, j-1))
                * tan(direction[i * max_control.rows + j])) || (grad.at<uchar>(i, j) <= (grad.at<uchar>(i, j+1)
                + (grad.at<uchar>(i+1, j+1) - grad.at<uchar>(i, j)) * tan(direction[i * max_control.rows + j])))){
                    max_control.at<uchar>(i, j) = 0;
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
    ui->label_3->resize(QTemp1.size());
    ui->label_3->show();

    QTemp2 = QImage((const uchar*)(max_control.data), max_control.cols, max_control.rows, max_control.cols * max_control.channels(), QImage::Format_Indexed8);
    ui->label_2->setPixmap(QPixmap::fromImage(QTemp2));
    QTemp2 = QTemp2.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setScaledContents(true);
    ui->label_2->resize(QTemp2.size());
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
    ui->label_3->resize(QTemp.size());
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
    ui->label_3->resize(QTemp.size());
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
                middle << guassianNoiseImg.at<Vec3b>(i-1, j-1)[k] << guassianNoiseImg.at<Vec3b>(i-1, j)[k]
                        << guassianNoiseImg.at<Vec3b>(i-1, j+1)[k] << guassianNoiseImg.at<Vec3b>(i, j-1)[k]
                        << guassianNoiseImg.at<Vec3b>(i, j)[k] << guassianNoiseImg.at<Vec3b>(i, j+1)[k]
                        << guassianNoiseImg.at<Vec3b>(i+1, j-1)[k] << guassianNoiseImg.at<Vec3b>(i+1, j)[k]
                        << guassianNoiseImg.at<Vec3b>(i+1, j+1)[k];
                std::sort(std::begin(middle), std::end(middle));
                filter_img.at<Vec3b>(i, j)[k] = saturate_cast<uchar>(middle[5]);
            }
        }
    }

    QTemp = QImage((const uchar*)(filter_img.data), filter_img.cols, filter_img.rows, filter_img.cols * filter_img.channels(), QImage::Format_Indexed8);
    ui->label_3->setPixmap(QPixmap::fromImage(QTemp));
    QTemp = QTemp.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_3->setScaledContents(true);
    ui->label_3->resize(QTemp.size());
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
    ui->label_3->resize(QTemp.size());
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
    ui->label_3->resize(QTemp.size());
    ui->label_3->show();
}

void MainWindow::on_frame_diff_clicked(){
    Mat pFrame, pFrame1, pFrame2, pFrame3;
    Mat pFrameGray1, pFrameGray2, pFrameGray3;
    Mat pFrameMat1, pFrameMat2;
    VideoCapture pCapture;
    int nFrame = 0;

    pCapture = VideoCapture("./video/test.mp4");
    pCapture >> pFrame;
    pFrame1.create(pFrame.size(), CV_8UC1);
    pFrame2.create(pFrame.size(), CV_8UC1);
    pFrame3.create(pFrame.size(), CV_8UC1);
    while(1){
        nFrame++;
        pCapture >> pFrame1;
        if (pFrame.data == NULL){
            return;
        }
        pCapture >> pFrame2;
        pCapture >> pFrame3;
        cvtColor(pFrame1, pFrameGray1, CV_BGR2GRAY);
        cvtColor(pFrame2, pFrameGray2, CV_BGR2GRAY);
        cvtColor(pFrame3, pFrameGray3, CV_BGR2GRAY);

        absdiff(pFrameGray1, pFrameGray2, pFrameMat1);
        absdiff(pFrameGray2, pFrameGray3, pFrameMat2);

        threshold(pFrameMat1, pFrameMat1, 10, 255, CV_THRESH_BINARY);
        threshold(pFrameMat2, pFrameMat2, 10, 255, CV_THRESH_BINARY);

        Mat element1 = getStructuringElement(0, cv::Size(3, 3));
        Mat element2 = getStructuringElement(0, cv::Size(5, 5));

        erode(pFrameMat1, pFrameMat1, element1);
        erode(pFrameMat2, pFrameMat2, element1);

        dilate(pFrameMat1, pFrameMat1, element2);
        dilate(pFrameMat2, pFrameMat2, element2);

        imshow("Frame Difference", pFrameMat2);

        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours(pFrameMat2, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
        double max_area = 0;
        int max_index = 0;
        for (size_t i = 0; i < contours.size(); i++){
            double area = contourArea(contours[i], false);
            if (max_area < area){
                max_area = area;
                max_index = i;
            }
        }
        if (max_index != 0){
            drawContours(pFrame2, contours, max_index, Scalar(0, 0, 255), 2);
        }

        imshow("Source Image", pFrame2);
        if (waitKey(1) != -1){
            break;
        }
    }
    pCapture.release();
    cv::destroyAllWindows();

}

void MainWindow::on_mix_gauss_clicked(){
    Mat gray_img, foreground1, foreground2, pFrame;
    Ptr<BackgroundSubtractorKNN> ptrKNN = createBackgroundSubtractorKNN(500, 400.0, true);
    Ptr<BackgroundSubtractorMOG2> ptrMOG2 = createBackgroundSubtractorMOG2(500, 16, true);
    namedWindow("Extracted Foreground");
    VideoCapture pCapture;
    pCapture = VideoCapture("./video/test.mp4");

    while (1){
        pCapture >> pFrame;
        if (pFrame.data == NULL){
            return;
        }
        cvtColor(pFrame, gray_img, CV_BGR2GRAY);
        long long t1 = getTickCount();
        ptrKNN->apply(pFrame, foreground1, -1);
        long long t2 = getTickCount();
        ptrMOG2->apply(pFrame, foreground2, -1);
        long long t3 = getTickCount();
        cout << "Time cost of KNN is " << t2 - t1 << "\n Time cost of MOG2 is " << t3 - t2 << endl;
        imshow("Source video frame", pFrame);
        imshow("Extracted foreground of KNN", foreground1);
        imshow("Extracted foreground of MOG2", foreground2);
        if (waitKey(1) != -1){
            break;
        }
    }
    cv::destroyAllWindows();
}

void MainWindow::on_circle_LBP_clicked(){
    QImage QTemp1, QTemp2, QTemp3, QTemp4;
    Mat Temp;
    int radius = 1, neighbors=8;

    Mat dst = Mat(grayImg.rows - 2 * radius, grayImg.cols - 2 * radius, CV_8UC1, Scalar(0));
    lbp_operator(grayImg, dst, radius, neighbors);
    QTemp1 = QImage((const uchar*)(dst.data), dst.cols, dst.rows, dst.cols*dst.channels(), QImage::Format_Indexed8);
    ui->label_2->setPixmap(QPixmap::fromImage(QTemp1));
    QTemp1 = QTemp1.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setScaledContents(true);
    ui->label_2->resize(QTemp1.size());
    ui->label_2->show();

}

void MainWindow::on_SIFT_clicked(){
    QString img_name1 = QFileDialog::getOpenFileName(this, tr(""), "./resources/img/", "files(*)");
    Mat src_img1 = imread(img_name1.toStdString());
    QString img_name2 = QFileDialog::getOpenFileName(this, tr(""), "./resources/img/", "files(*)");
    Mat src_img2 = imread(img_name2.toStdString());
    imshow("Source image 1", src_img1);
    imshow("Source image 2", src_img2);
    Ptr<SIFT> SIFT_detector = SIFT::create();
    vector<KeyPoint> kp1, kp2;
    Mat descriptor1, descriptor2, res1, res2, trans_img1, trans_img2;
    char str1[20], str2[20];

    SIFT_detector->detect(src_img1, kp1);
    SIFT_detector->compute(src_img1, kp1, descriptor1);
    drawKeypoints(src_img1, kp1, res1);
    trans_img1 = src_img1.clone();
    sprintf(str1, "%zd", kp1.size());
    putText(trans_img1, str1, Point(280, 230), 0, 1.0, Scalar(255, 0, 0), 2);
    imshow("Descriptor ", trans_img1);

    SIFT_detector->detect(src_img2, kp2);
    SIFT_detector->compute(src_img2, kp2, descriptor2);
    drawKeypoints(src_img2, kp2, res2);
    trans_img2 = src_img2.clone();
    sprintf(str2, "%zd", kp2.size());
    putText(trans_img2, str2, Point(280, 230), 0, 1.0, Scalar(255, 0, 0), 2);
    imshow("Descriptor ", trans_img2);

    BFMatcher matcher(NORM_L2, true);
    vector<DMatch> matches;
    matcher.match(descriptor1, descriptor2, matches);
    Mat img_match;
    drawMatches(src_img1, kp1, src_img2, kp2, matches, img_match);
    imshow("Match", img_match);
    waitKey(0);
    cv::destroyAllWindows();
    waitKey(1);

}

void MainWindow::on_orb_clicked() {
    QString object_name, scene_name;
    Mat object_img, scene_img;
    do {
        object_name = QFileDialog::getOpenFileName(this, tr(""), "./resources/img/", "files(*)");
        object_img = imread(object_name.toStdString());
        cout << "Error Object Image!! Please choose a new object image again!!" << endl;
    } while ((object_img.empty()));

    do {
        scene_name = QFileDialog::getOpenFileName(this, tr(""), "./resources/img/", "files(*)");
        scene_img = imread(scene_name.toStdString());
        cout << "Error Scene Image!! Please choose a new scene image again!!" << endl;
    } while ((scene_img.empty()));

    vector<KeyPoint> object_keypoints, scene_keypoints;
    Mat object_descriptor, scene_descriptor;
    Ptr<ORB> orb_detector = ORB::create();

    orb_detector->detect(object_img, object_keypoints);
    orb_detector->compute(object_img, object_keypoints, object_descriptor);
    orb_detector->detect(scene_img, scene_keypoints);
    orb_detector->compute(scene_img, scene_keypoints, scene_descriptor);

    // use the hamming filter to compute the similarity
    BFMatcher bf_matcher(NORM_HAMMING, true);
    vector<DMatch> matches;
    bf_matcher.match(object_descriptor, scene_descriptor, matches);
    Mat matched_img;
    drawMatches(object_img, object_keypoints, scene_img, scene_keypoints, matches, matched_img);
    imshow("Matched Image", matched_img);

    // save the matched-pair index
    vector<int> query_indexes(matches.size()), train_indexes(matches.size());
    for (size_t i = 0; i < matches.size(); i++) {
        query_indexes[i] = matches[i].queryIdx;
        train_indexes[i] = matches[i].trainIdx;
    }

    // transform matrix
    Mat H12;
    vector<Point2f> points1, points2;
    KeyPoint::convert(object_keypoints, points1, query_indexes);
    KeyPoint::convert(scene_keypoints, points2, train_indexes);

    int ransacReprojThreshold = 5;
    H12 = findHomography(Mat(points1), Mat(points2), RANSAC, ransacReprojThreshold);
    // mask
    vector<char> matchesMask(matches.size(), 0);
    Mat points1t;
    perspectiveTransform(Mat(points1), points1t, H12);
    for (size_t i = 0; i < points1.size(); i++){
        if (norm(points2[i] - points1t.at<Point2f>((int)i, 0)) <= ransacReprojThreshold) matchesMask[i] = 1;
    }

    Mat matched_img_after_mask;
    drawMatches(object_img, object_keypoints, scene_img, scene_keypoints, matches, matched_img_after_mask,
                Scalar(0, 0, 255), Scalar::all(-1), matchesMask);
    vector<Point2f> object_corners(4), scene_corners(4);
    object_corners[0] = Point(0, 0);
    object_corners[1] = Point(object_img.cols, 0);
    object_corners[2] = Point(object_img.cols, object_img.rows);
    object_corners[3] = Point(0, object_img.rows);
    perspectiveTransform(object_corners, scene_corners, H12);
    line(matched_img_after_mask, Point2f((scene_corners[0].x + static_cast<float>(object_img.cols)), scene_corners[0].y),
         Point2f((scene_corners[1].x + static_cast<float>(object_img.cols)), scene_corners[1].y), Scalar(0, 0, 255), 2);
    line(matched_img_after_mask, Point2f((scene_corners[1].x + static_cast<float>(object_img.cols)), scene_corners[1].y),
         Point2f((scene_corners[2].x + static_cast<float>(object_img.cols)), scene_corners[2].y), Scalar(0, 0, 255), 2);
    line(matched_img_after_mask, Point2f((scene_corners[2].x + static_cast<float>(object_img.cols)), scene_corners[2].y),
         Point2f((scene_corners[3].x + static_cast<float>(object_img.cols)), scene_corners[3].y), Scalar(0, 0, 255), 2);
    line(matched_img_after_mask, Point2f((scene_corners[3].x + static_cast<float>(object_img.cols)), scene_corners[3].y),
         Point2f((scene_corners[0].x + static_cast<float>(object_img.cols)), scene_corners[0].y), Scalar(0, 0, 255), 2);

    float a_tan = atan(abs((scene_corners[3].y - scene_corners[0].y) / (scene_corners[3].x - scene_corners[0].x)));
    float pi = atan(1) * 4;
    a_tan = 90 - 180 * a_tan / pi;
    imshow("Match Image after remove mask points", matched_img_after_mask);
    imshow("Scene Image", scene_img);

    waitKey(0);
    cv::destroyAllWindows();
    waitKey(1);

}

void MainWindow::on_haar_vertical_clicked(){
    if (grayImg.empty()){
        cvtColor(srcImg, grayImg, COLOR_BGR2GRAY);
    }
    Mat sum_img = Mat::zeros(grayImg.rows + 1, grayImg.cols + 1, CV_32FC1);
    integral(grayImg, sum_img, CV_64F);

    int step_x = 8, step_y = 8;
    Rect rect1, rect2;
    Mat haar_img = Mat::zeros(srcImg.size(), CV_32FC1);
    for (int i = 0; i < srcImg.rows; i = i + step_x){
        for (int j = 0; j < srcImg.cols; j = j + step_y){
            rect1 = Rect(j, i, 2, 4);
            rect2 = Rect(j+2, i, 2, 4);
            haar_img.at<float>(i, j) = sum_of_rect(grayImg, rect1) - sum_of_rect(grayImg, rect2);
        }
    }

    QImage QTemp1;
    QTemp1 = QImage((const uchar*)(haar_img.data), haar_img.cols, haar_img.rows, haar_img.cols*haar_img.channels(), QImage::Format_Indexed8);
    ui->label_2->setPixmap(QPixmap::fromImage(QTemp1));
    QTemp1 = QTemp1.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setScaledContents(true);
    ui->label_2->resize(QTemp1.size());
    ui->label_2->show();

}

void MainWindow::on_haar_horizontal_clicked() {
    if (grayImg.empty()) {
        cvtColor(srcImg, grayImg, COLOR_BGR2GRAY);
    }
    Mat sum_img = Mat::zeros(grayImg.rows + 1, grayImg.cols + 1, CV_32FC1);
    integral(grayImg, sum_img, CV_64F);

    int step_x = 8, step_y = 8;
    Rect rect1, rect2;
    Mat haar_img = Mat::zeros(srcImg.size(), CV_32FC1);
    for (int i = 0; i < srcImg.rows; i = i + step_x) {
        for (int j = 0; j < srcImg.cols; j = j + step_y) {
            rect1 = Rect(j, i, 2, 4);
            rect2 = Rect(j, i + 2, 2, 4);
            haar_img.at<float>(i, j) = sum_of_rect(grayImg, rect1) - sum_of_rect(grayImg, rect2);
        }
    }

    QImage QTemp1;
    QTemp1 = QImage((const uchar *) (haar_img.data), haar_img.cols, haar_img.rows, haar_img.cols * haar_img.channels(),
                    QImage::Format_Indexed8);
    ui->label_3->setPixmap(QPixmap::fromImage(QTemp1));
    QTemp1 = QTemp1.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_3->setScaledContents(true);
    ui->label_3->resize(QTemp1.size());
    ui->label_3->show();
}

void MainWindow::on_haar_face_clicked() {
    CascadeClassifier face_cascade;
    face_cascade.load("./sources/haarcascade_frontalface_alt2.xml");
    VideoCapture video_capture;
    video_capture.open(0);
    if (! video_capture.isOpened()){
        cout << "No camera is detected! You can choose a already video instead of the direct camera!" << endl;
    }
    QString video_name = QFileDialog::getOpenFileName(this, tr(""), "./resources/video/", "files(*)");
    video_capture.open(video_name.toStdString(), CAP_ANY);
    Mat img, gray_img;
    vector<Rect> faces;
    while (true){
        video_capture >> img;
        if (img.empty()) {
            continue;
        }
        if (img.channels() == 3) {
            cvtColor(img, gray_img, CV_BGR2GRAY);
        }
        else if (img.channels() == 3) {
            gray_img = img;
        }
        else throw "Image channel is not matched. Please use the video with three channels or one channel";
        // detect the face from the image
        face_cascade.detectMultiScale(gray_img, faces, 1.2, 6, 0, Size(0, 0));
        if (faces.size() > 0) {
            for (int i = 0; i < faces.size(); i++) {
                rectangle(gray_img, Point(faces[i].x, faces[i].y), Point(faces[i].x + faces[i].width, faces[i].y +
                        faces[i].height), Scalar(0, 255, 0), 1, 8);
                putText(gray_img, "Face", Point(faces[i].x, faces[i].y - 5), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0));

            }
            imshow("Face", gray_img);
        }
        else cout << "No face is detected!!!";
        if (waitKey(1) != -1) break;
    }
    cv::destroyWindow("Face");
}


void MainWindow::on_color_fit_clicked() {
    QString img_name = QFileDialog::getOpenFileName(this, tr(""), "./resources/img/", "files(*)");
    Mat src_img = imread(img_name.toStdString());
    if (!src_img.data || src_img.channels() != 3) throw "Error Image or Error Image Channels!!!";
    String window_name = "Source Image";
    namedWindow(window_name, WINDOW_AUTOSIZE);
    imshow(window_name, src_img);
    // hue
    int hsv_hmin = 0, hsv_hmin_max = 360, hsv_hmax = 360, hsv_hmax_max = 360;
    // saturation
    int hsv_smin = 0, hsv_smin_max = 255, hsv_smax = 255, hsv_smax_max = 255;
    // value
    int hsv_vmin = 106, hsv_vmin_max = 255, hsv_vmax = 255, hsv_vmax_max = 255;
    Mat norm_img, hsv_img;
    src_img.convertTo(norm_img, CV_32FC1, 1.0 / 255, 0);
    cvtColor(norm_img, hsv_img, COLOR_BGR2HSV);
    window_name = "HSV Image";
    namedWindow(window_name, WINDOW_GUI_EXPANDED);
    // adjust the hue
    createTrackbar("hmin", window_name, &hsv_hmin, hsv_hmin_max);
    createTrackbar("hmax", window_name, &hsv_hmax, hsv_hmax_max);
    // adjust the saturation
    createTrackbar("smin", window_name, &hsv_smin, hsv_smin_max);
    createTrackbar("smax", window_name, &hsv_smax, hsv_smax_max);
    // adjust the value
    createTrackbar("vmin", window_name, &hsv_vmin, hsv_vmin_max);
    createTrackbar("vmax", window_name, &hsv_vmax, hsv_vmax_max);
    Mat dst_img = Mat::zeros(src_img.size(), CV_32FC3), mask;
    inRange(hsv_img, Scalar(hsv_hmin, hsv_smin / float(hsv_smin_max), hsv_vmin / float(hsv_vmin_max)),
            Scalar(hsv_hmax, hsv_smax / float(hsv_smax_max), hsv_vmax / float(hsv_vmax_max)), mask);
    for (int i = 0; i < norm_img.rows; i++) {
        for (int j = 0; j < norm_img.cols; j++) {
            if (mask.at<uchar>(i, j) == 255) {
                dst_img.at<Vec3f>(i, j) = norm_img.at<Vec3f>(i, j);
            }
        }
    }
    imshow("HSV Fit Image", dst_img);
    waitKey(0);
    cv::destroyAllWindows();
    waitKey(1);
}

void MainWindow::on_svm_clicked() {
    int width = 512, height = 512;
    Mat src_img = Mat(height, width, CV_8UC3, Scalar(0, 255, 255));

    double labels[5] = {1.0, -1.0, -1.0, -1.0, 1.0};
    Mat labels_mat(5, 1, CV_32SC1, labels);
    float training_data[5][2] = { {501, 300}, {255, 10}, {501, 255}, {10, 501}, {450, 500}};
    Mat training_data_mat(5, 2, CV_32FC1, training_data);

    Ptr<ml::SVM> svm = ml::SVM::create();
    svm->setType(ml::SVM::C_SVC);
    svm->setKernel(ml::SVM::POLY);
    svm->setDegree(1.0);
    svm->setTermCriteria(TermCriteria(CV_TERMCRIT_ITER, 100, 1e-6));
    svm->train(training_data_mat, ml::SampleTypes::ROW_SAMPLE, labels_mat);
    svm->save("./sources/mnist_svm.xml");

    // testing
    Mat sample_mat;
    Vec3b red(0, 0, 255), green(0, 255, 0), blue(255, 0, 0);
    for (int i = 0; i < src_img.rows; i++) {
        for (int j = 0; j < src_img.cols; j++) {
            sample_mat = (Mat_<float>(1, 2) << j, i);
            float f_response = svm->predict(sample_mat);
            if (f_response == 1) src_img.at<Vec3b>(i, j) = green;
            else if (f_response == -1) src_img.at<Vec3b>(i, j) = blue;
            if (i > 525 - 0.5 * j) src_img.at<Vec3b>(i, j) = green;
            else src_img.at<Vec3b>(i, j) = blue;
        }
    }

    // show the data
    int thickness = -1, line_type = 8;
    for (int i = 0; i < training_data_mat.rows; i++) {
        if (labels[i] == 1) circle(src_img, Point(training_data[i][0], training_data[i][1]), 5, Scalar(0, 0, 0), thickness, line_type);
        else circle(src_img, Point(training_data[i][0], training_data[i][1]), 5, Scalar(255, 255, 255), thickness, line_type);
    }

    thickness = 2;
    line_type = 8;
    Mat vector = svm->getSupportVectors();
    int var_count = svm->getVarCount();
    for (int i = 0; i < vector.rows; i++) {
        int x = (int)vector.at<float>(i, 0);
        int y = (int)vector.at<float>(i, 1);
        circle(src_img, Point(x, y), 6, Scalar(0, 0, 255), thickness, line_type);
    }

    imshow("circle", src_img);
    waitKey(0);
    destroyAllWindows();
    waitKey(1);
}

void MainWindow::on_word_clicked() {
    Ptr<ml::SVM> svm = ml::SVM::load("./sources/SVM_HOG.xml");
    if (svm->empty()) throw "Fail to load the svm detector!!!";

    QString img_name = QFileDialog::getOpenFileName(this, tr(""), "./resources/img/", "files(*)");
    Mat src_img = imread(img_name.toStdString());
    if (!src_img.data || src_img.channels() != 3) throw "Error Image or Error Image Channels!!!";
    cv::resize(src_img, src_img, Size(28, 28), 1);
    imshow("Resource Image", src_img);

    HOGDescriptor hog(Size(14, 14), Size(7, 7), Size(1, 1), Size(7, 7), 9);
    vector<float> img_descriptor;
    hog.compute(src_img, img_descriptor, Size(5, 5));
    Mat sample_mat;
    sample_mat.create(1, img_descriptor.size(), CV_32FC1);

    for (int i = 0; i < img_descriptor.size(); i++) {
        sample_mat.at<float>(0, i) = img_descriptor[i];
    }
    int result_word = svm->predict(sample_mat);
    cout << "Result word is " << result_word << endl;
    waitKey(0);
    waitKey(1);

}