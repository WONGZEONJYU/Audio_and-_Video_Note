//
// Created by wong on 2024/8/27.
//

// You may need to build the project (run Qt uic code generator) to get "ui_sdl_qt_rgb.h" resolved

#include "sdl_qt_rgb.hpp"
#include "ui_sdl_qt_rgb.h"
#include <SDL.h>
#include <QDebug>
#include <QMessageBox>
#include <QImage>

#undef main

sdl_qt_rgb::sdl_qt_rgb(QWidget *parent) :
        QWidget(parent), ui(new Ui::sdl_qt_rgb) {

    ui->setupUi(this);

    m_sdl_w = ui->label->width();
    m_sdl_h = ui->label->height();

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    m_screen = SDL_CreateWindowFrom(reinterpret_cast<void*>(ui->label->winId()));

    m_renderer = SDL_CreateRenderer(m_screen,-1,SDL_RENDERER_ACCELERATED);

    QImage img1("001.png"),img2("002.png");
    if (img1.isNull() || img2.isNull()){
        QMessageBox::warning(this,"","open image failed!");
        return;
    }

    const auto out_w{img1.width() + img2.width()}; //两张图的宽度相加
    const auto out_h{img1.height() > img2.height() ? img1.height() : img2.height()};
    //以高度最高的为准
    m_sdl_w = out_w;
    m_sdl_h = out_h;
    resize(m_sdl_w,m_sdl_h);
    ui->label->move(0,0);
    ui->label->resize(m_sdl_w,m_sdl_h);

    m_texture = SDL_CreateTexture(m_renderer,
                      SDL_PIXELFORMAT_ARGB8888,
                      SDL_TEXTUREACCESS_STREAMING,
                                  m_sdl_w,m_sdl_h);

    m_rgb_datum.resize(m_sdl_w * m_sdl_h * m_pix_size,0);
    auto dst_{m_rgb_datum.data()};

    for (int h {}; h < m_sdl_h; ++h) {

        auto w_Offset {h * m_sdl_w * m_pix_size};
        //行偏移一次字节大小m_sdl_w = img1.width() + img2.width() (m_pix_size是像素点大小)

        if (h < img1.height()) {
            const auto img1_width{img1.width() * m_pix_size};
            std::copy_n(img1.scanLine(h),img1_width,dst_ + w_Offset); //拷贝img1一行的数据
        }

        w_Offset += img1.width() * m_pix_size; //第一个图像读取完一行数据,对地址再次偏移img1.width() (m_pix_size是像素点大小)

        if (h < img2.height()){
            const auto img2_width{img2.width() * m_pix_size};
            std::copy_n(img2.scanLine(h),img2_width,dst_ + w_Offset); //拷贝img2一行的数据
        }
    }

    QImage out(m_rgb_datum.data(),m_sdl_w,m_sdl_h,QImage::Format_ARGB32);
    out.save("out.png");
    startTimer(10);
}

sdl_qt_rgb::~sdl_qt_rgb() {
    delete ui;
    SDL_DestroyTexture(m_texture);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_screen);
}

void sdl_qt_rgb::timerEvent(QTimerEvent *) {

    const auto dst_{m_rgb_datum.data()};
#if 0
    if (!(--m_count)) {
        m_count = 255;
        m_rgb[m_i++] = 0;
        m_i %= std::size(m_rgb);
        m_rgb[m_i] = 0xff;
    }

    --m_rgb[m_i];


    for (uint32_t h {}; h < m_sdl_h; ++h) {
        const auto drift{ h * m_sdl_w * m_pix_size };
        for (uint32_t w {}; w < m_sdl_w * m_pix_size; w += m_pix_size) {
            //dst_[drift + w] = m_rgb[2];       //B
            dst_[drift + w + 1] = m_rgb[1];   //G
            //dst_[drift + w + 2] = m_rgb[0];   //R
            //dst_[drift + w + 3] = 0;        //A
        }
    }
#endif

    if (SDL_UpdateTexture(m_texture, nullptr, dst_ ,m_sdl_w * m_pix_size) < 0) { //更新纹理
        qDebug() << __LINE__ << ": " << SDL_GetError();
        return;
    }

    if (SDL_RenderClear(m_renderer) < 0) { //清除渲染器
        qDebug() << __LINE__ << ": " << SDL_GetError() << "\n";
        return ;
    }

#if WIN64
    const SDL_Rect rect{0,0,m_sdl_w,m_sdl_h};
#else
    const auto pos{ui->label->pos()};
    const SDL_Rect rect{pos.x(),pos.y(),m_w,m_h};
#endif

    if (SDL_RenderCopy(m_renderer,m_texture, nullptr, &rect) < 0) { //纹理数据拷贝到渲染器
        qDebug() << __LINE__ << ": " << SDL_GetError() << "\n";
        return ;
    }

    SDL_RenderPresent(m_renderer); //开始渲染
}
