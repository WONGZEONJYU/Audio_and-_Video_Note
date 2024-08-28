//
// Created by wong on 2024/8/27.
//

// You may need to build the project (run Qt uic code generator) to get "ui_sdl_qt_rgb.h" resolved

#include "sdl_qt_rgb.hpp"
#include "ui_sdl_qt_rgb.h"
#include <SDL.h>
#include <QMessageBox>
#include <QImage>
#include "xvideo_view.hpp"

#undef main

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
    m_view->Init(m_sdl_w,m_sdl_h,XVideoView::YUV420P,reinterpret_cast<void*>(ui->label->winId()));
    //m_view->Init(m_sdl_w,m_sdl_h,XVideoView::YUV420P, nullptr);

//    m_screen = SDL_CreateWindowFrom(reinterpret_cast<void*>(ui->label->winId()));
//    m_renderer = SDL_CreateRenderer(m_screen,-1,SDL_RENDERER_ACCELERATED);
//
//    ui->label->move(0,0);
//    ui->label->resize(m_sdl_w,m_sdl_h);
//
//    m_texture = SDL_CreateTexture(m_renderer,
//                      SDL_PIXELFORMAT_IYUV, // /**< Planar mode: Y + U + V  (3 planes) */
//                      SDL_TEXTUREACCESS_STREAMING,
//                                  m_sdl_w,m_sdl_h);

    m_yuv_datum.resize(m_sdl_w * m_sdl_h * m_pix_size,0);

    //startTimer(10);
}

sdl_qt_rgb::~sdl_qt_rgb() {
    delete ui;
//    SDL_DestroyTexture(m_texture);
//    SDL_DestroyRenderer(m_renderer);
//    SDL_DestroyWindow(m_screen);
    delete m_view;
    m_yuv_file.close();
}

void sdl_qt_rgb::timerEvent(QTimerEvent *e) {

//    if (m_yuv_file.atEnd()){
//        killTimer(e->timerId());
//        QMessageBox::warning(this,"","yuv_file end!");
//        return;
//    }

    //const auto dst_{m_yuv_datum.data()};

    //m_yuv_file.read(reinterpret_cast<char*>(dst_),static_cast<qint64>(m_sdl_w * m_sdl_h * 1.5));
    //读取YUV数据,一帧YUY数据是 m_sdl_w * m_sdl_h + m_sdl_w * m_sdl_h / 4  + m_sdl_w * m_sdl_h / 4
    // m_sdl_w * m_sdl_h + m_sdl_w/2 * m_sdl_h/2 + m_sdl_w/2 * m_sdl_h/2
    // 化简公式 m_sdl_w * m_sdl_h * 1.5
    //此处不涉及行对齐问题,行对齐问题在后面有

//    if (SDL_UpdateTexture(m_texture, nullptr, dst_ ,m_sdl_w ) < 0) { //更新纹理
//        QMessageBox::warning(this,QString::number(__LINE__),SDL_GetError());
//        return;
//    }
//
//    if (SDL_RenderClear(m_renderer) < 0) { //清除渲染器
//        QMessageBox::warning(this,QString::number(__LINE__),SDL_GetError());
//        return ;
//    }

//#if WIN64
//    const SDL_Rect rect{0,0,m_sdl_w,m_sdl_h};
//#else
//    const auto pos{ui->label->pos()};
//    const SDL_Rect rect{pos.x(),pos.y(),m_w,m_h};
//#endif

//    if (SDL_RenderCopy(m_renderer,m_texture, nullptr, &rect) < 0) { //纹理数据拷贝到渲染器
//        QMessageBox::warning(this,QString::number(__LINE__),SDL_GetError());
//        return ;
//    }
//
//    SDL_RenderPresent(m_renderer); //开始渲染
}
