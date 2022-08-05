//
// Created by Svain on 2022/8/3.
//

#include "MainWindow.h"
#include "QApplication"

int main(int argc, char *argv[]){
    vector<vector<Point>> contours;

    QApplication imgProc(argc, argv);
    MainWindow main_window;
    main_window.show();
    return imgProc.exec();
}