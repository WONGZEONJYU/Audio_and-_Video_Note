//
// Created by wong on 2024/8/27.
//

// You may need to build the project (run Qt uic code generator) to get "ui_sdl_qt_rgb.h" resolved

#include "sdl_qt_rgb.hpp"
#include "ui_sdl_qt_rgb.h"
#include <QMessageBox>
#include <QResizeEvent>
#include "xvideo_view.hpp"

#include "XHelper.hpp"
#include <QStringList>
#include <QFileDialog>

#undef main

#if MACOS
extern "C" void setMetalViewFrame(void *winId, int x,int y,int width, int height);

void adjustMetalViewFrame(QWidget *qtWidget) {
    void *winId = reinterpret_cast<void *>(qtWidget->winId());
    setMetalViewFrame(winId, qtWidget->x(),qtWidget->y(),qtWidget->width(), qtWidget->height());
}

// YourCppFile.cpp
extern "C" void addMetalViewToParent(void *parentWinId, void *metalWinId);

void integrateMetalViewWithQt(QWidget *parentWidget, QWidget *metalWidget) {
    void *parentWinId = reinterpret_cast<void *>(parentWidget->winId());
    void *metalWinId = reinterpret_cast<void *>(metalWidget->winId());

    // 调用封装的 Objective-C++ 函数
    addMetalViewToParent(parentWinId, metalWinId);
}

#endif

sdl_qt_rgb::sdl_qt_rgb(QWidget *parent) :
        QWidget(parent), ui(new Ui::sdl_qt_rgb) {

    ui->setupUi(this);
    connect(this,&sdl_qt_rgb::ViewS,this,&sdl_qt_rgb::View,Qt::UniqueConnection);
    m_views.push_back(XVideoView::create());
    m_views.push_back(XVideoView::create());
    m_views[0]->Set_Win_ID(reinterpret_cast<void *>(ui->video1->winId()));
    m_views[1]->Set_Win_ID(reinterpret_cast<void *>(ui->video2->winId()));
#if MACOS
    adjustMetalViewFrame(ui->video1);
    adjustMetalViewFrame(ui->video2);
    integrateMetalViewWithQt(this, ui->video1);
    integrateMetalViewWithQt(this,ui->video2);
#endif
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
        XVideoView::MSleep(10);
    }
}

void sdl_qt_rgb::View() {
    //主线程调用本函数
    static QLabel* const fps_s[]{ui->view_fps1,ui->view_fps2};

    static int64_t last_pts[32]{},fps_arr[std::size(last_pts)]{};

    fps_arr[0] = ui->set_fps1->value();
    fps_arr[1] = ui->set_fps2->value();

    for (uint32_t i{};i < m_views.size();++i) {

        const auto ms{1000LL / fps_arr[i]},
                    curr_time{XVideoView::Get_time_ms()};

        if (curr_time - last_pts[i] < ms) {
            continue;
        }
        last_pts[i] = curr_time;

        auto item{m_views[i]};
        auto frame{item->Read()};
        if (frame) {
            item->DrawFrame(frame);
            QStringList ss(GET_STR(FPS: ) + QString::number(item->Render_Fps()));
            fps_s[i]->setText(ss.join(GET_STR()));
        }
    }

#if MACOS
    ui->view_fps1->raise();
    ui->view_fps2->raise();
    ui->height1->raise();
    ui->height2->raise();
    ui->open1->raise();
    ui->open2->raise();
    ui->set_fps1->raise();
    ui->set_fps2->raise();
    ui->pix1->raise();
    ui->pix2->raise();
    ui->width1->raise();
    ui->width2->raise();
#endif
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
    if (!view->Open(file_name.toLocal8Bit().toStdString())){
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
    } else if (GET_STR(RGB24) == pix){
        fmt = XVideoView::RGB24;
    }else{}

    //初始化窗口和纹理
    view->Init(w,h,fmt);
}
