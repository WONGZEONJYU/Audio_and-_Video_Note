
#ifndef XHELPER_H
#define XHELPER_H

#include <string>
#include <memory>
#include <atomic>
#include <system_error>
#include <type_traits>
#include <iostream>

#ifdef HAVE_FFMPEG
struct AVFormatContext;
struct AVPacket;
struct AVFilterGraph;
struct AVChannelLayout;
struct AVRational;
class XAVPacket;
class XAVFrame;
class XCodecParameters;
struct AVCodecParameters;
using XCodecParameters_sp = std::shared_ptr<XCodecParameters>;
using XAVPacket_sp = std::shared_ptr<XAVPacket>;
using XAVFrame_sp = std::shared_ptr<XAVFrame>;
struct AVCodecContext;
struct XRational {
    int num{1}, ///< Numerator
    den{1}; ///< Denominator
};

#endif

#if HAVE_SDL2
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
#endif

enum XLogLevel{
    XLOG_TYPE_DEBUG,
    XLOG_TYPE_INFO,
    XLOG_TYPE_ERROR,
    XLOG_TYPE_FATAL,
};

namespace XHelper {
#ifdef HAVE_FFMPEG
    std::string av_get_err(const int&) noexcept(true);

    void log_packet(const AVFormatContext &, const AVPacket &)  noexcept(true);

    void av_filter_graph_dump(AVFilterGraph *,const std::string &) noexcept(false);

    void check_ff_func(const std::string &func,const std::string &file,
                       const int &line,const int &err_code) noexcept(false);

    void ff_err_out(const std::string &func,const std::string &file,
                    const int &line,const int &err_code) noexcept(true);

    std::string channel_layout_describe(const AVChannelLayout &) noexcept(true);
#endif

#ifdef HAVE_OPENGL
    void checkOpenGLError(const std::string & , const std::string & ,const int &) noexcept(true);
#endif

#ifdef HAVE_SDL2
    void sdl2_err_out(const std::string &func,const std::string &file,
                    const int &line) noexcept(true);
#endif

    void check_nullptr(const std::string &func,const std::string &file,
                       const int &line,const void *p) noexcept(false);

    bool is_nullptr(const std::string &func,const std::string &file,
                    const int &line,const void *p) noexcept(true);

    void check_EXC(const std::string &func,const std::string &file,
                   const int &line,const std::exception &e) noexcept(false);

    std::error_code make_error_code_helper(const int &errcode) noexcept(true);

    void print_err_tips(const std::string &func,const std::string &file,
                        const int &line,const std::string &msg) noexcept(true);

    bool float_Compare(const float &,const float &,const float & = 1e-6);

    uint64_t Get_time_ms();

    void MSleep(const uint64_t &);

    void xlog(const std::string &func,
              const std::string &file,
              const int &line,
              const std::string &msg,
              const int &level = XLOG_TYPE_DEBUG);
}

#define LOGDEBUG(msg) XHelper::xlog(__func__,__FILE__,__LINE__,(msg))
#define LOGDINFO(msg) XHelper::xlog(__func__,__FILE__,__LINE__,(msg),XLOG_TYPE_INFO)
#define LOGERROR(msg) XHelper::xlog(__func__,__FILE__,__LINE__,(msg),XLOG_TYPE_ERROR)
#define LOGFATAL(msg) XHelper::xlog(__func__,__FILE__,__LINE__,(msg),XLOG_TYPE_FATAL)

#ifdef HAVE_FFMPEG
    #define FF_CHECK_ERR(x,...) do{\
    const auto _err_code_{x};\
    if(_err_code_ < 0){\
        __VA_ARGS__;\
        XHelper::check_ff_func(#x,__FILE__,__LINE__,_err_code_);\
    }}while(false)

    #define FF_ERR_OUT(x,...) do{\
        const auto _err_code_{x};\
        if(_err_code_ < 0){\
            XHelper::ff_err_out(#x,__FILE__,__LINE__,_err_code_);\
            __VA_ARGS__;\
        }\
    }while(false)

#endif

#ifdef HAVE_OPENGL
#define GL_CHECK(x) do { \
        x;\
        XHelper::checkOpenGLError(#x, __FILE__, __LINE__); \
    } while (false)
#endif

#ifdef HAVE_SDL2

#define SDL2_PTR_ERR_OUT(x,...) do{ \
    auto _p_ {static_cast<const void * const>(x)};\
    if(!_p_){\
       XHelper::sdl2_err_out(#x,__FILE__, __LINE__);\
       __VA_ARGS__;\
    }\
}while(false)

#define SDL2_INT_ERR_OUT(x,...) do{ \
    const auto _ret_{x};\
    if(_ret_ < 0){\
       XHelper::sdl2_err_out(#x,__FILE__, __LINE__);\
       __VA_ARGS__;\
    }\
}while(false)

#endif

#define CHECK_NULLPTR(x,...) do{ \
    const auto _p_ {x};\
    static_assert(std::is_pointer_v<std::remove_cv_t<decltype(_p_)>>,#x);\
    try{\
        XHelper::check_nullptr(#x,__FILE__,__LINE__,static_cast<const void*>(_p_));\
    }catch(...){\
        __VA_ARGS__;\
        throw;\
    }\
}while(false)

#define IS_NULLPTR(x,...) do{ \
    const auto _p_ {x};           \
    static_assert(std::is_pointer_v<std::remove_cv_t<decltype(_p_)>>,#x); \
    if(!XHelper::is_nullptr(#x,__FILE__,__LINE__,static_cast<const void*>(_p_))){ \
        __VA_ARGS__;\
    }\
}while(false)

#define CHECK_EXC(x,...)do{ \
    try{x;}catch(const std::exception &e){ \
    __VA_ARGS__;\
    XHelper::check_EXC(#x,__FILE__,__LINE__,e);}\
}while(false)

#define PRINT_ERR_TIPS(msg) do{ \
    XHelper::print_err_tips(__func__,__FILE__,__LINE__,(msg));}while(false)

#define GET_STR(args) #args


#define TRY_CATCH(x,...) do{ \
       try{x;}catch(const std::exception &e){ \
            std::cerr << e.what() << "\n";    \
            __VA_ARGS__;\
       }\
}while(false)

#define X_DISABLE_COPY(Class) \
    Class(const Class &) = delete;\
    Class &operator=(const Class &) = delete;

#define X_DISABLE_COPY_MOVE(Class) \
    X_DISABLE_COPY(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete;

template<typename F>
struct Destroyer final{
    X_DISABLE_COPY(Destroyer)
    inline explicit Destroyer(F &&f):fn(std::move(f)){}
    inline void destroy() {
        if (!is_destroy) {
            is_destroy = true;
            fn();
        }
    }

    ~Destroyer() {
        destroy();
    }

private:
    F fn;
    std::atomic_bool is_destroy{};
};

template<typename F1,typename F2>
struct XRAII final {
    X_DISABLE_COPY(XRAII)
    explicit XRAII(F1 &&f1,F2 &&f2) : m_f2(std::move(f2)){
        f1();
    }

    void destroy(){
        if (!m_is_destroy){
            m_is_destroy = true;
            m_f2();
        }
    }

    ~XRAII(){
        destroy();
    }

private:
    F2 m_f2;
    std::atomic_bool m_is_destroy{};
};

#endif