//
// Created by wong on 2024/8/27.
//

// You may need to build the project (run Qt uic code generator) to get "ui_sdl_qt_rgb.h" resolved

#include "sdl_qt_rgb.hpp"
#include "ui_sdl_qt_rgb.h"
#include <SDL.h>
#include <QDebug>

sdl_qt_rgb::sdl_qt_rgb(QWidget *parent) :
        QWidget(parent), ui(new Ui::sdl_qt_rgb) {

    ui->setupUi(this);

    m_w = ui->label->width();
    m_h = ui->label->height();

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    m_screen = SDL_CreateWindowFrom(reinterpret_cast<void*>(ui->label->winId()));

    m_renderer = SDL_CreateRenderer(m_screen,-1,SDL_RENDERER_ACCELERATED);

    m_texture = SDL_CreateTexture(m_renderer,
                      SDL_PIXELFORMAT_ARGB8888,
                      SDL_TEXTUREACCESS_STREAMING,
                      m_w,m_h);

    m_rgb_datum.resize(m_h * m_w * 4);

    startTimer(10);
}

sdl_qt_rgb::~sdl_qt_rgb() {
    delete ui;
    SDL_DestroyTexture(m_texture);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_screen);
}

void sdl_qt_rgb::timerEvent(QTimerEvent *) {

    if (SDL_RenderClear(m_renderer) < 0) { //清除渲染器
        qDebug() << __LINE__ << ": " << SDL_GetError() << "\n";
        return ;
    }

    if (!(--m_count)) {
        m_count = 255;
        m_rgb[m_i++] = 0;
        m_i %= std::size(m_rgb);
        m_rgb[m_i] = 0xff;
    }

    --m_rgb[m_i];

    const auto dst_{m_rgb_datum.data()};
    for (uint32_t h {}; h < m_h; ++h) {
        const auto drift{ h * m_w * 4 };
        for (uint32_t w {}; w < m_w * 4; w += 4) {
            dst_[drift + w] = m_rgb[2];       //B
            dst_[drift + w + 1] = m_rgb[1];   //G
            dst_[drift + w + 2] = m_rgb[0];   //R
            dst_[drift + w + 3] = 0;        //A
        }
    }

    if (SDL_UpdateTexture(m_texture, nullptr, dst_ ,m_w * 4) < 0) { //更新纹理
        qDebug() << __LINE__ << ": " << SDL_GetError();
        return;
    }

    const auto pos{ui->label->pos()};
    const SDL_Rect rect{pos.x(),pos.y(),m_w,m_h};

    if (SDL_RenderCopy(m_renderer,m_texture, nullptr, &rect) < 0) { //纹理数据拷贝到渲染器
        qDebug() << __LINE__ << ": " << SDL_GetError() << "\n";
        return ;
    }

    SDL_RenderPresent(m_renderer); //开始渲染
}
