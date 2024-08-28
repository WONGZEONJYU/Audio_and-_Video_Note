//
// Created by Administrator on 2024/8/28.
//

#include "xsdl.hpp"
#include <SDL.h>

bool XSDL::Init(const int &w,const int &h,const Format &fmt,void *winID) {

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    m_width = w;
    m_height = h;
    m_fmt = fmt;
    m_winID = winID;

    m_win = m_winID ? SDL_CreateWindowFrom(m_winID) : SDL_CreateWindow("",
                                                                       SDL_WINDOWPOS_CENTERED,
                                                                       SDL_WINDOWPOS_CENTERED,
                                                                       m_width,
                                                                       m_height,
                                                                       SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    if (!m_win){
        TRY_CATCH(CHECK_NULLPTR(m_win));
        return {};
    }

    return true;
}

bool XSDL::Draw(const void *datum, const int &line_size) {

    return true;
}

XSDL::~XSDL() {
    SDL_DestroyTexture(m_texture);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_win);
}
