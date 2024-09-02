//
// Created by wong on 2024/8/27.
//

// You may need to build the project (run Qt uic code generator) to get "ui_sdl_qt_rgb.h" resolved

#include "sdl_qt_rgb.hpp"
#include "ui_sdl_qt_rgb.h"
#include <QMessageBox>
#include <QResizeEvent>
#include "xvideo_view.hpp"
#include "XAVFrame.hpp"
#include "XHelper.hpp"
#include <QStringList>
#include <QFileDialog>

#undef main

sdl_qt_rgb::sdl_qt_rgb(QWidget *parent) :
        QWidget(parent), ui(new Ui::sdl_qt_rgb) {

    ui->setupUi(this);
    (void )connect(this,&sdl_qt_rgb::ViewS, this,&sdl_qt_rgb::View,Qt::UniqueConnection);
    m_views.push_back(XVideoView::create());
    m_views.push_back(XVideoView::create());
    m_views[0]->Set_Win_ID(reinterpret_cast<void *>(ui->video1->winId()));
    m_views[1]->Set_Win_ID(reinterpret_cast<void *>(ui->video2->winId()));
    m_th = std::thread(&sdl_qt_rgb::Main, this);
}

sdl_qt_rgb::~sdl_qt_rgb() {
    m_th_is_exit = true;
    if (m_th.joinable()){
        m_th.join();
    }
    for (auto &item :m_views) {
        delete item;
        item = nullptr;
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

    auto view{m_views[index]};
    //打开文件
    if (!view->Open(file_name.toStdString())){
        qDebug() << "open file error!";
        return;
    }
    qDebug() << file_name.toLocal8Bit();

    int w{},h{};
    QString pix;
    switch (index) {
        case 0:
            w = ui->width1->value();
            h = ui->height1->value();
            pix = ui->pix1->currentText();
            break;
        case 1:
            w = ui->width2->value();
            h = ui->height2->value();
            pix = ui->pix2->currentText();
            break;
        default:
            break;
    }

    auto fmt{XVideoView::YUV420P};
    if (GET_STR(YUV420P) == pix){
        fmt = XVideoView::YUV420P;
    } else if (GET_STR(RGBA) == pix){
        fmt = XVideoView::RGBA;
    }else if (GET_STR(ARGB) == pix){
        fmt = XVideoView::ARGB;
    } else if (GET_STR(BGRA) == pix){
        fmt = XVideoView::BGRA;
    } else{}

    //初始化窗口和材质
    view->Init(w,h,fmt);

}
