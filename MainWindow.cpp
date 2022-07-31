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
    QImage Qtemp;
    cvtColor(srcImg, temp, CV_BGR2RGB);
    Qtemp = QImage((const unsigned char *)(temp.data), temp.cols, temp.rows, temp.step, QImage::Format_RGB888);

    Ui->label->setPixmap(QPixmap::fromImage(Qtemp));
    Qtemp = Qtemp.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    Ui->label->setScaledContents(true);
    Ui->label->resize(Qtemp.size);
    Ui->label->show;

}

void on_select_images_clicked();

void on_gray_hist_clicked();

void on_gray_balance_clicked();

void on_gray_sharpen_clicked();

void on_laplacian_sharpen_clicked();

void on_add_salt_noise_clicked();

void on_add_guassian_noise_clicked();

void on_roberts_edge_detection_clicked();

void on_sobel_edge_detection_clicked();

void on_prewitt_edge_detection_clicked();

void on_laplacian_edge_detection_clicked();

void on_krisch_edge_detection_clicked();

void on_canny_edge_detection_clicked();