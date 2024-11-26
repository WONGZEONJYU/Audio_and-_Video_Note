#include "xsdl.hpp"
#include <SDL.h>

using namespace std;

static inline bool sdl_init() {
    static atomic_bool is_init{};
    if (!is_init){
        static mutex mux;
        unique_lock locker(mux);
        SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
        SDL2_INT_ERR_OUT(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO),return {});
        //direct3d
        //SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d");
        //设置缩放算法,解决锯齿问题,采用双线性插值算法
        CHECK_FALSE_(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"1"),PRINT_ERR_TIPS(SDL_GetError());return {});
        //IS_FALSE_(SDL_SetHint(SDL_HINT_VIDEO_WINDOW_SHARE_PIXEL_FORMAT,"1"),PRINT_ERR_TIPS(SDL_GetError());return {});
        is_init = true;
    }
    return is_init;
}

bool XSDL::Init(const int &w,const int &h,const Format &fmt,const string &win_title) {

    if (w <= 0 || h <= 0){
        PRINT_ERR_TIPS("const int &w,const int &h error!");
        return {};
    }

    if (!sdl_init()){
        return {};
    }

    m_width_ = w;
    m_height_ = h;
    m_fmt_ = fmt;

    unique_lock locker(m_mux_);

    if (m_texture_){
        SDL_DestroyTexture(m_texture_);
    }

    if (m_renderer_){
        SDL_DestroyRenderer(m_renderer_);
    }

    if (!m_win_) { //窗口不能重复创建
        SDL2_PTR_ERR_OUT(m_win_ = m_winID_ ? SDL_CreateWindowFrom(m_winID_) : SDL_CreateWindow(win_title.c_str(),
                                                                       SDL_WINDOWPOS_CENTERED,
                                                                       SDL_WINDOWPOS_CENTERED,
                                                                       m_width_,m_height_,
                                                                       SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL),return {});

        //SDL_SetWindowOpacity(m_win_, 0.5f); // 设置 SDL 窗口半透明
    }

    SDL2_PTR_ERR_OUT(m_renderer_ = SDL_CreateRenderer(m_win_,-1,SDL_RENDERER_ACCELERATED),//使用硬件渲染,如有不支持,可以替换
                     return {}); //创建渲染器

    SDL2_INT_ERR_OUT(SDL_SetRenderDrawBlendMode(m_renderer_, SDL_BLENDMODE_BLEND),return {});
    SDL2_INT_ERR_OUT(SDL_SetRenderDrawColor(m_renderer_, 0, 0, 0, 255),return {}); // 设置透明背景

    auto pix_fmt{SDL_PIXELFORMAT_RGBA32};
    m_pixel_Byte_size_ = 4;
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
        case ABGR:
            pix_fmt = SDL_PIXELFORMAT_ABGR32;
            break;
        case YUV420P:
            pix_fmt = SDL_PIXELFORMAT_IYUV;
            m_pixel_Byte_size_ = 1;
            break;
        case RGB24:
            pix_fmt = SDL_PIXELFORMAT_RGB24;
            m_pixel_Byte_size_ = 3;
            break;
        case BGR24:
            pix_fmt = SDL_PIXELFORMAT_BGR24;
            m_pixel_Byte_size_ = 3;
            break;
        case NV12:
            pix_fmt = SDL_PIXELFORMAT_NV12;
            m_pixel_Byte_size_ = 1;
        case NV21:
            pix_fmt = SDL_PIXELFORMAT_NV21;
            m_pixel_Byte_size_ = 1;
            break;
        default:
            break;
    }

    SDL2_PTR_ERR_OUT(m_texture_ = SDL_CreateTexture(m_renderer_,
                                                   pix_fmt,
                                                   SDL_TEXTUREACCESS_STREAMING, //纹理需频繁改变,带锁
                                                   m_width_,m_height_),return {});
    return true;
}

bool XSDL::Draw(const void *datum,int line_size) {

    if (!datum) {
        PRINT_ERR_TIPS(GET_STR(datum is nullptr));
        return {};
    }

    unique_lock locker(m_mux_);

    if (!check_init()) { //SDL初始化检查
        return {};
    }

    if (line_size <= 0) {
#if 1
        line_size = m_width_ * m_pixel_Byte_size_;
#else
        switch (m_fmt) {
            case RGB24:
                line_size = m_width_ * 3;
                break;
            case RGBA:
            case BGRA:
            case ARGB:
                line_size = m_width_ * 4;
                break;
            case YUV420P:
            case NV12:
                line_size = m_width_;
                break;
            default:
                PRINT_ERR_TIPS(GET_STR(line_size error!));
                return {};
        }
#endif
    }

    SDL2_INT_ERR_OUT(SDL_UpdateTexture(m_texture_, nullptr,datum,line_size),return {});
    return Start_Rendering();
}

bool XSDL::Draw(const uint8_t *y,const int &y_pitch,
                const uint8_t *u,const int &u_pitch,
                const uint8_t *v,const int &v_pitch) {

    if (!y || !u || !v || y_pitch <= 0 || u_pitch <= 0 || v_pitch <= 0){ //输入参数有误
        PRINT_ERR_TIPS(GET_STR(Parameter error!));
        return {};
    }

    if(!check_init()) { //SDL初始化检查
        return {};
    }

    unique_lock locker(m_mux_);

    SDL2_INT_ERR_OUT(SDL_UpdateYUVTexture(m_texture_,nullptr,y,y_pitch,u,u_pitch,v,v_pitch),return {});
    //更新YUV420到纹理

    return Start_Rendering();
}

bool XSDL::Draw(const uint8_t *y,const int &y_pitch,
          const uint8_t *uv,const int &uv_pitch) {

    if (!y || !uv || y_pitch <= 0 || uv_pitch <= 0){
        PRINT_ERR_TIPS(GET_STR(Parameter error!));
        return {};
    }

    if(!check_init()) { //SDL初始化检查
        return {};
    }

    unique_lock locker(m_mux_);
    SDL2_INT_ERR_OUT(SDL_UpdateNVTexture(m_texture_, nullptr,y,y_pitch,uv,uv_pitch),return {});
    //更新NV12 / NV21到纹理

    return Start_Rendering();
}

XSDL::~XSDL() {
    XSDL::Close();
}

void XSDL::Close() {
    unique_lock locker(m_mux_);
    DeConstruct();
}

/**
 * 此函数没有锁,需谨慎使用
 */
void XSDL::DeConstruct(){
    if (m_texture_) {
        SDL_DestroyTexture(m_texture_);
        m_texture_ = {};
    }

    if (m_renderer_) {
        SDL_DestroyRenderer(m_renderer_);
        m_renderer_ = {};
    }

    if (m_win_) {
        SDL_DestroyWindow(m_win_);
        m_win_ = {};
    }
}

bool XSDL::Is_Exit_Window() const {
    SDL_Event event{};
    SDL2_INT_ERR_OUT(SDL_WaitEventTimeout(&event,1),return {});
    return SDL_QUIT == event.type;
}

void XSDL::Scale(const int &w,const int &h) {
    XVideoView::Scale(w, h);
}

bool XSDL::Start_Rendering() const {

    SDL2_INT_ERR_OUT(SDL_RenderClear(m_renderer_),return {});
    //清理渲染器
#if 1
    SDL_Rect rect{};
    const SDL_Rect *p_rect{};
    if (m_scale_w_ > 0 || m_scale_h_ > 0) { //更改纹理宽高
        rect.w = m_scale_w_;
        rect.h = m_scale_h_;
        p_rect = std::addressof(rect);
    }

    SDL2_INT_ERR_OUT(SDL_RenderCopyEx(m_renderer_,m_texture_, nullptr,p_rect,{},{},SDL_FLIP_NONE),return {});
    //拷贝纹理数据到渲染器
#else
    SDL_FRect fRect{};
    const SDL_FRect *p_fRect{};
    if (m_scale_w > 0 || m_scale_h > 0) {
        fRect.w = static_cast<float >(m_scale_w);
        fRect.h = static_cast<float >(m_scale_h);
    }
    SDL2_INT_ERR_OUT(SDL_RenderCopyExF(m_renderer_,m_texture_,{},p_fRect,{},{},SDL_FLIP_NONE),return {});
#endif
    SDL_RenderPresent(m_renderer_); //开始渲染
    return true;
}

bool XSDL::check_init() const{

    const auto b {!m_win_ || !m_renderer_ || !m_texture_ || m_width_ <= 0 || m_height_ <= 0};
    if (b){
        PRINT_ERR_TIPS(GET_STR(Uninitialized));
    }
    return !b;
}

void XSDL::ShowWindow(){
    unique_lock lock(m_mux_);
    SDL_ShowWindow(m_win_);
}
