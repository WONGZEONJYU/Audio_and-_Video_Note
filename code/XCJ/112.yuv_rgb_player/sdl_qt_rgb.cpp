//
// Created by wong on 2024/8/27.
//

// You may need to build the project (run Qt uic code generator) to get "ui_sdl_qt_rgb.h" resolved

#include "sdl_qt_rgb.hpp"
#include "ui_sdl_qt_rgb.h"
#include <QMessageBox>
#include <QImage>
#include <QResizeEvent>
#include "xvideo_view.hpp"
#include "XAVFrame.hpp"
#include "XHelper.hpp"
#include <QStringList>
#include <QSpinBox>
#include <QFile>
#include <QFileDialog>

#undef main

sdl_qt_rgb::sdl_qt_rgb(QWidget *parent) :
        QWidget(parent), ui(new Ui::sdl_qt_rgb) {

    ui->setupUi(this);
    connect(this,&sdl_qt_rgb::ViewS, this,&sdl_qt_rgb::View,Qt::UniqueConnection);

    m_th = std::thread(&sdl_qt_rgb::Main, this);
}

sdl_qt_rgb::~sdl_qt_rgb() {
    m_th_is_exit = true;
    if (m_th.joinable()){
        m_th.join();
    }
    delete ui;
}

void sdl_qt_rgb::timerEvent(QTimerEvent *e) {

}

void sdl_qt_rgb::resizeEvent(QResizeEvent *e) {

    QWidget::resizeEvent(e);
}

void sdl_qt_rgb::Main() {
    while (!m_th_is_exit){
        ViewS();
        XHelper::MSleep(1);
    }
}

void sdl_qt_rgb::View(){

}

void sdl_qt_rgb::Open1() {
    qDebug() << __func__ ;
    Open(0);
}

void sdl_qt_rgb::Open2() {
    qDebug() << __func__ ;
    Open(1);
}

void sdl_qt_rgb::Open(const int &index){

    const auto file_name{QFileDialog::getOpenFileName()};
    if (file_name.isEmpty()){
        qDebug() << "file_name is empty";
        return;
    }

    qDebug() << file_name.toLocal8Bit();
}
