//
// Created by wong on 2024/8/27.
//

// You may need to build the project (run Qt uic code generator) to get "ui_sdl_qt_rgb.h" resolved

#include "sdl_qt_rgb.hpp"
#include "ui_sdl_qt_rgb.h"
#include <SDL.h>
#include <QDebug>

#undef main

sdl_qt_rgb::sdl_qt_rgb(QWidget *parent) :
        QWidget(parent), ui(new Ui::sdl_qt_rgb) {

    ui->setupUi(this);

    m_sdl_w = ui->label->width();
    m_sdl_h = ui->label->height();

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    m_screen = SDL_CreateWindowFrom(reinterpret_cast<void*>(ui->label->winId()));
    /*SDL窗口创建来源于QT窗口*/

    m_renderer = SDL_CreateRenderer(m_screen,-1,SDL_RENDERER_ACCELERATED);
    /*创建渲染器*/

    m_texture = SDL_CreateTexture(m_renderer,
                      SDL_PIXELFORMAT_ARGB8888,/*纹理像素格式,注意的是,他的内存排布*/
                      SDL_TEXTUREACCESS_STREAMING, /*频繁更改纹理内容参数*/
                      m_sdl_w,m_sdl_h);
    /*创建纹理*/

    m_rgb_datum.resize(m_sdl_h * m_sdl_w * m_pix_size);

    startTimer(10);
}

sdl_qt_rgb::~sdl_qt_rgb() {
    delete ui;
    SDL_DestroyTexture(m_texture);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_screen);
}

void sdl_qt_rgb::timerEvent(QTimerEvent *) {

    if (!(--m_count)) {
        m_count = 255;
        m_rgb[m_i++] = 0;
        m_i %= std::size(m_rgb);
        m_rgb[m_i] = 0xff;
    }

    --m_rgb[m_i];

    const auto dst_{m_rgb_datum.data()};
    for (uint32_t h {}; h < m_sdl_h; ++h) {
        const auto drift{ h * m_sdl_w * m_pix_size };
        for (uint32_t w {}; w < m_sdl_w * m_pix_size; w += m_pix_size) {
            dst_[drift + w] = m_rgb[2];       //B
            dst_[drift + w + 1] = m_rgb[1];   //G
            dst_[drift + w + 2] = m_rgb[0];   //R
            dst_[drift + w + 3] = 0;        //A
        }
    }

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
    const SDL_Rect rect{pos.x(),pos.y(),m_sdl_w,m_sdl_h};
#endif

    if (SDL_RenderCopy(m_renderer,m_texture, nullptr, &rect) < 0) { //纹理数据拷贝到渲染器
        qDebug() << __LINE__ << ": " << SDL_GetError() << "\n";
        return ;
    }

    SDL_RenderPresent(m_renderer); //开始渲染
}
