//
// Created by wong on 2024/8/27.
//

// You may need to build the project (run Qt uic code generator) to get "ui_sdl_qt_rgb.h" resolved

#include "sdl_qt_rgb.hpp"
#include "ui_sdl_qt_rgb.h"
#include <SDL.h>
#include <QMessageBox>
#include <QImage>
#include <QResizeEvent>
#include "xvideo_view.hpp"

#undef main

static void yuvMirror(uint8_t* yuv,const int &w, const int &h);

sdl_qt_rgb::sdl_qt_rgb(QWidget *parent) :
        QWidget(parent), ui(new Ui::sdl_qt_rgb) {

    ui->setupUi(this);
    m_yuv_file.setFileName("400_300_25.yuv");
    if(!m_yuv_file.open(QFile::ReadOnly)){
        QMessageBox::warning(this,"","open yuv_file error!");
        return;
    }

    m_sdl_w = 400;
    m_sdl_h = 300;

    m_view = XVideoView::create();

    //m_view->Init(m_sdl_w,m_sdl_h,XVideoView::YUV420P);
    m_view->Init(m_sdl_w,m_sdl_h,XVideoView::YUV420P,reinterpret_cast<void*>(ui->label->winId()));

    m_yuv_datum.resize(m_sdl_w * m_sdl_h * m_pix_size,0);
    startTimer(10);
}

sdl_qt_rgb::~sdl_qt_rgb() {
    delete ui;
    delete m_view;
    m_yuv_file.close();
}

void sdl_qt_rgb::timerEvent(QTimerEvent *e) {

    if (m_yuv_file.atEnd()){
        killTimer(e->timerId());
        QMessageBox::warning(this,"","yuv_file end!");
        return;
    }

    if (m_view->Is_Exit_Window()){
        m_view->Close();
        close();
        return;
    }

    const auto dst_{m_yuv_datum.data()};

    m_yuv_file.read(reinterpret_cast<char*>(dst_),
                    static_cast<qint64>(m_sdl_w * m_sdl_h * 1.5));
    //读取YUV数据,一帧YUY数据是 m_sdl_w * m_sdl_h + m_sdl_w * m_sdl_h / 4  + m_sdl_w * m_sdl_h / 4
    // m_sdl_w * m_sdl_h + m_sdl_w/2 * m_sdl_h/2 + m_sdl_w/2 * m_sdl_h/2
    // 化简公式 m_sdl_w * m_sdl_h * 1.5
    //此处不涉及行对齐问题,行对齐问题在后面有

    yuvMirror(dst_,m_sdl_w,m_sdl_h);

    m_view->Draw(dst_);
}

void sdl_qt_rgb::resizeEvent(QResizeEvent *e) {
    ui->label->resize(size());
    ui->label->move({});
    m_view->Scale(width(),height());
    QWidget::resizeEvent(e);
}

static void yuvMirror(uint8_t* yuv,const int &w, const int &h) {

    constexpr auto swap_{[](uint8_t &a, uint8_t &b){
        a ^= b;
        b ^= a;
        a ^= b;
#if 0
        //等价于
        uint8_t temp{a};
        a = b;
        b = temp;
#endif
    }};

    for (int i {};i < h;++i) {
        auto a{i * w},b{(i + 1) * w - 1};
        while (a < b){
            swap_(yuv[a], yuv[b]);
            a++;b--;
        }
    }

    const auto uStartPos{w * h};
    for (int i{};i < (h / 2);++i) {
        auto a{i * w / 2},b{(i + 1) * w / 2 - 1};
        while (a < b){
            swap_(yuv[a+ uStartPos], yuv[b+ uStartPos]);
            a++;b--;
        }
    }

    const auto vStartPos{w * h / 4 * 5};
    for (int i{};i < (h / 2);++i) {
        auto a{i * w / 2},b{(i + 1) * w / 2 - 1};
        while (a < b){
            swap_(yuv[a+ vStartPos], yuv[b+ vStartPos]);
            a++;b--;
        }
    }
}
