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
        SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
        SDL2_INT_ERR_OUT(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO),return {});
        if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"1") < SDL_FALSE){
            //设置缩放算法,解决锯齿问题,采用双线性插值算法
            PRINT_ERR_TIPS(SDL_GetError());
            return {};
        }
        is_init = true;
    }
    return is_init;
}

bool XSDL::Init(const int &w,const int &h,const Format &fmt) {

    if (w <= 0 || h <= 0){
        PRINT_ERR_TIPS("const int &w,const int &h error!");
        return {};
    }

    sdl_init();

    m_width = w;
    m_height = h;
    m_fmt = fmt;

    unique_lock locker(m_mux);

    if (m_texture){
        SDL_DestroyTexture(m_texture);
    }
    if (m_renderer){
        SDL_DestroyRenderer(m_renderer);
    }

    if (!m_win) { //窗口不能重复创建
        SDL2_PTR_ERR_OUT(m_win = m_winID ? SDL_CreateWindowFrom(m_winID) : SDL_CreateWindow("",
                                                                       SDL_WINDOWPOS_CENTERED,
                                                                       SDL_WINDOWPOS_CENTERED,
                                                                       m_width,m_height,
                                                                       SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL),return {});
        //SDL_SetWindowOpacity(m_win, 0.5f); // 设置 SDL 窗口半透明
    }

    SDL2_PTR_ERR_OUT(m_renderer = SDL_CreateRenderer(m_win,-1,SDL_RENDERER_ACCELERATED),//使用硬件渲染,如有不支持,可以替换
                     return {});
    //创建渲染器
    SDL2_INT_ERR_OUT(SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND),return {});
    SDL2_INT_ERR_OUT(SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255),return {}); // 设置透明背景

    auto pix_fmt{SDL_PIXELFORMAT_RGBA32};
    m_pixel_Byte_size = 4;
    switch (fmt) {
        case RGBA:
            pix_fmt = SDL_PIXELFORMAT_RGBA32;
            break;
        case BGRA:
            pix_fmt = SDL_PIXELFORMAT_BGRA32;
            break;
        case ARGB:
            pix_fmt = SDL_PIXELFORMAT_ARGB32;
            break;
        case YUV420P:
            pix_fmt = SDL_PIXELFORMAT_IYUV;
            m_pixel_Byte_size = 1;
            break;
        case RGB24:
            pix_fmt = SDL_PIXELFORMAT_RGB24;
            m_pixel_Byte_size = 3;
            break;
        default:
            break;
    }

    SDL2_PTR_ERR_OUT(m_texture = SDL_CreateTexture(m_renderer,
                                                   pix_fmt,
                                                   SDL_TEXTUREACCESS_STREAMING,
                                                   m_width,m_height),return {});

    return true;
}

bool XSDL::Draw(const void *datum, int line_size) {

    if (!datum) {
        PRINT_ERR_TIPS(GET_STR(datum is nullptr));
        return {};
    }

    unique_lock locker(m_mux);

    if (!check_init()) { //SDL初始化检查
        return {};
    }

    if (line_size <= 0) {
        line_size = m_width * m_pixel_Byte_size;
#if 0
        switch (m_fmt) {
            case RGB24:
                line_size = m_width * 3;
                break;
            case RGBA:
            case BGRA:
            case ARGB:
                line_size = m_width * 4;
                break;
            case YUV420P:
                line_size = m_width;
                break;
            default:
                PRINT_ERR_TIPS(GET_STR(line_size error!));
                return {};
        }
#endif
    }

    SDL2_INT_ERR_OUT(SDL_UpdateTexture(m_texture,{},datum,line_size),return {});
    return Start_Rendering();
}

bool XSDL::Draw(const uint8_t *y, int y_pitch,
                const uint8_t *u, int u_pitch,
                const uint8_t *v, int v_pitch) {

    auto b {!y || !u || !v || y_pitch <= 0 || u_pitch <= 0 || v_pitch <= 0};
    if (b){ //输入参数有误
        PRINT_ERR_TIPS(GET_STR(Parameter error!));
        return {};
    }

    if(!check_init()) { //SDL初始化检查
        return {};
    }

    unique_lock locker(m_mux);

    SDL2_INT_ERR_OUT(SDL_UpdateYUVTexture(m_texture,{},y,y_pitch,u,u_pitch,v,v_pitch),return {});
    //更新YUV到纹理

    return Start_Rendering();
}

XSDL::~XSDL() {
    DeConstruct();
}

void XSDL::Close() {
    unique_lock locker(m_mux);
    DeConstruct();
}

void XSDL::DeConstruct(){
    if (m_texture) {
        SDL_DestroyTexture(m_texture);
        m_texture = {};
    }

    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = {};
    }

    if (m_win) {
        SDL_DestroyWindow(m_win);
        m_win = {};
    }
}

bool XSDL::Is_Exit_Window() const {
    SDL_Event event{};
    SDL2_INT_ERR_OUT(SDL_WaitEventTimeout(&event,1),return {});
    return SDL_QUIT == event.type;
}

void XSDL::Scale(const int &w,const int &h) {
    if (m_winID){
        XVideoView::Scale(w, h);
    }
}

bool XSDL::Start_Rendering() {

    SDL2_INT_ERR_OUT(SDL_RenderClear(m_renderer),return {});
    //清理渲染器
#if 1
    SDL_Rect rect{};
    const SDL_Rect *p_rect{};
    if (m_scale_w > 0 || m_scale_h > 0) { //更改纹理宽高
        rect.w = m_scale_w;
        rect.h = m_scale_h;
        p_rect = std::addressof(rect);
    }
    SDL2_INT_ERR_OUT(SDL_RenderCopyEx(m_renderer,m_texture,{},p_rect,{},{},SDL_FLIP_NONE),return {});
    //拷贝纹理数据到渲染器
#else
    SDL_FRect fRect{};
    const SDL_FRect *p_fRect{};
    if (m_scale_w > 0 || m_scale_h > 0) {
        fRect.w = static_cast<float >(m_scale_w);
        fRect.h = static_cast<float >(m_scale_h);
    }
    SDL2_INT_ERR_OUT(SDL_RenderCopyExF(m_renderer,m_texture,{},p_fRect,{},{},SDL_FLIP_NONE),return {});
#endif
    SDL_RenderPresent(m_renderer); //开始渲染
    return true;
}

bool XSDL::check_init(){

    const auto b {!m_win || !m_renderer || !m_texture || m_width <= 0 || m_height <= 0};
    if (b){
        PRINT_ERR_TIPS(GET_STR(Uninitialized));
    }
    return !b;
}
