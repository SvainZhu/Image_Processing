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
    QString img_name = QFileDialog::getOpenFileName(this, tr(""), "../img/", "files(*)");
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
    QString img_name1 = QFileDialog::getOpenFileName(this, tr(""), "../img/", "files(*)");
    Mat src_img1 = imread(img_name1.toStdString());
    QString img_name2 = QFileDialog::getOpenFileName(this, tr(""), "../img/", "files(*)");
    Mat src_img2 = imread(img_name2.toStdString());
    imshow("Source image 1", src_img1);
    imshow("Source image 2", src_img2);
    Ptr<SIFT> SIFT_detector = SIFT::create();
    vector<KeyPoint> kp1, kp2;
    Mat descriptor1, descriptor2, res1, res2, trans_img1, trans_img2;
    char str1[20], str2[20];

    SIFT_detector->detect(src_img1, kp1);
    SIFT_detector->compute(src_img1, descriptor1);
    drawKeypoints(src_img1, kp1, res1);
    trans_img1 = src_img1.clone();
    sprintf(str1, "%d", kp1.size());
    putText(trans_img1, str1, Point(280, 230), 0, 1.0, Scalar(255, 0, 0), 2);
    imshow("Descriptor ", trans_img1);

    SIFT_detector->detect(src_img2, kp2);
    SIFT_detector->compute(src_img2, descriptor2);
    drawKeypoints(src_img2, kp2, res2);
    trans_img2 = src_img2.clone();
    sprintf(str2, "%d", kp2.size());
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