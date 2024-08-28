//
// Created by Administrator on 2024/8/28.
//

#include "xsdl.hpp"
#include <SDL.h>

using namespace std;

static inline bool sdl_init(){
    static atomic_bool is_init{};

    if (!is_init){
        static mutex mux;
        unique_lock locker(mux);
        SDL2_INT_ERR_OUT(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO),return {});
        is_init = true;
    }
    return is_init;
}

bool XSDL::Init(const int &w,const int &h,const Format &fmt,void *winID) {

    if (w <= 0 || h <= 0){
        return {};
    }

    sdl_init();

    m_width = w;
    m_height = h;
    m_fmt = fmt;
    m_winID = winID;

    unique_lock locker(m_mux);

    if (!m_win){
        SDL2_PTR_ERR_OUT(m_win = winID ? SDL_CreateWindowFrom(winID) : SDL_CreateWindow("",
                                                                                        SDL_WINDOWPOS_CENTERED,
                                                                                        SDL_WINDOWPOS_CENTERED,
                                                                                        m_width,
                                                                                        m_height,
                                                                                        SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL),return {});
    }

    if (!m_renderer){
        SDL2_PTR_ERR_OUT(m_renderer = SDL_CreateRenderer(m_win,-1,SDL_RENDERER_ACCELERATED),return {});
    }

    auto pix_fmt{SDL_PIXELFORMAT_RGBA8888};
    if (!m_texture){
        SDL2_PTR_ERR_OUT(m_texture = SDL_CreateTexture(m_renderer,
                                                       pix_fmt,
                                                       SDL_TEXTUREACCESS_STREAMING,
                                                       m_width,m_height),return {});
    }

    return true;
}

bool XSDL::Draw(const void *datum, const int &line_size) {

    SDL2_INT_ERR_OUT(SDL_UpdateTexture(m_texture, nullptr,datum,line_size),return {});

    SDL2_INT_ERR_OUT(SDL_RenderClear(m_renderer),return {});

    const SDL_Rect rect{0,0,m_width,m_height};

    SDL2_INT_ERR_OUT(SDL_RenderCopy(m_renderer,m_texture,nullptr,&rect),return {});

    SDL_RenderPresent(m_renderer); //开始渲染

    return true;
}

XSDL::~XSDL() {
    SDL_DestroyTexture(m_texture);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_win);
}
