//
// Created by SvainZhu on 2022/5/19.
//
#include "mainwindow.h"
#include "ui_mainwindow.h"

// init main window
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);
}

//deploy the class of MainWindow
MainWindow::~MainWindow(){
    delete ui;
}

Mat
