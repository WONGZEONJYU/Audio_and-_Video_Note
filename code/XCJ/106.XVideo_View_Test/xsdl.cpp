//
// Created by Administrator on 2024/8/28.
//

#include "xsdl.hpp"
#include <SDL.h>

bool XSDL::Init(const int &w,const int &h,const Format &fmt,void *winID) {

    m_width = w;
    m_height = h;
    m_fmt = fmt;
    m_winID = winID;

    m_win = SDL_CreateWindow()

    return true;
}

bool XSDL::Draw(const void *datum, const int &line_size) {




    return true;
}
