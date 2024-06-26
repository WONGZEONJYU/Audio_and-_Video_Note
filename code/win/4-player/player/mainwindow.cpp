//
// Created by wong on 2024/6/20.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "mainwindow.hpp"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::construct() noexcept(false) {

}

MainWindow_sptr new_MainWindow() noexcept(false)
{
    try {
        MainWindow_sptr obj{new MainWindow};
        obj->construct();
        return obj;
    } catch (const std::bad_alloc &e) {
        throw std::runtime_error("new MainWindow failed\n");
    } catch (const std::runtime_error &e) {
        throw e;
    }
}
