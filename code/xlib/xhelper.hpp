#ifndef XHELPER_HPP_
#define XHELPER_HPP_

#include <string>
#include <memory>
#include <atomic>
#include <vector>
#include <list>
#include <mutex>
#include <fstream>
#include <algorithm>
#include <system_error>
#include <type_traits>
#include <iostream>
#include <thread>

#ifdef _WIN32
    #ifdef XLIB_EXPORTS
        #define XLIB_API __declspec(dllexport)
    #else
        #define XLIB_API __declspec(dllimport)
    #endif
#else
#define XLIB_API
#endif

#ifdef HAVE_FFMPEG
struct AVChannelLayout;
struct AVFormatContext;
struct AVPacket;
struct AVFilterGraph;
struct AVChannelLayout;
struct AVRational;
struct AVCodecParameters;
struct AVCodecContext;
struct SwsContext;
struct SwsFilter;
struct SwrContext;
class XAVPacket;
class XAVFrame;
class XCodecParameters;
class XSwscale;
class XSwrSample;
using XCodecParameters_sp = std::shared_ptr<XCodecParameters>;
using XAVPacket_sp = std::shared_ptr<XAVPacket>;
using XAVFrame_sp = std::shared_ptr<XAVFrame>;
using XSwscale_sp = std::shared_ptr<XSwscale>;
using XSwrSample_sp = std::shared_ptr<XSwrSample>;
struct XRational {
    int num{1}, ///< Numerator
    den{1}; ///< Denominator
};
#endif

class XVideoView;
using XVideoView_sp = std::shared_ptr<XVideoView>;

#ifdef HAVE_SDL2
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

namespace XHelper  {
#ifdef HAVE_FFMPEG

    XLIB_API auto XRescale(const int64_t &pts,
    const AVRational &src_tb,const AVRational &dst_tb) ->int64_t;

    XLIB_API auto XRescale(const int64_t &pts,
        const XRational &src_tb,
        const XRational &dst_tb) ->int64_t;

    XLIB_API std::string av_get_err(const int&) noexcept(true);

    XLIB_API void log_packet(const AVFormatContext &, const AVPacket &)  noexcept(true);

    XLIB_API void av_filter_graph_dump(AVFilterGraph *,const std::string &) noexcept(false);

    XLIB_API void check_ff_func(const std::string &func,const std::string &file,
                       const int &line,const int &err_code) noexcept(false);

    XLIB_API void ff_err_out(const std::string &func,const std::string &file,
                    const int &line,const int &err_code) noexcept(true);

    XLIB_API std::string channel_layout_describe(const AVChannelLayout &) noexcept(true);
#endif

#ifdef HAVE_OPENGL
    XLIB_API void checkOpenGLError(const std::string & , const std::string & ,const int &) noexcept(true);
#endif

#ifdef HAVE_SDL2
    XLIB_API void sdl2_err_out(const std::string &func,const std::string &file,
                    const int &line) noexcept(true);
#endif

    XLIB_API void check_nullptr(const std::string &func,const std::string &file,
                       const int &line,const void *p) noexcept(false);

    XLIB_API bool is_Nullptr_(const std::string &func,const std::string &file,
                    const int &line,const void *p) noexcept(true);

    XLIB_API bool is_false(const std::string &func,const std::string &file,
        const int &line,const bool &b) noexcept(true);

    XLIB_API void check_EXC(const std::string &func,const std::string &file,
                   const int &line,const std::exception &e) noexcept(false);

    XLIB_API std::error_code make_error_code_helper(const int &errcode) noexcept(true);

    XLIB_API void print_err_tips(const std::string &func,const std::string &file,
                        const int &line,const std::string &msg) noexcept(true);

    XLIB_API uint64_t Get_time_ms();

    XLIB_API void MSleep(const uint64_t &);

    XLIB_API void x_log(const std::string &func,
              const std::string &file,
              const int &line,
              const std::string &msg,
              const int &level = XLOG_TYPE_DEBUG);

    static inline auto present_thread_id(){
        using std::this_thread::get_id;
        return get_id();
    }
}

#define LOG_DEBUG(msg) XHelper::x_log(__FUNCTION__,__FILE__,__LINE__,(msg))
#define LOG_INFO(msg) XHelper::x_log(__FUNCTION__,__FILE__,__LINE__,(msg),XLOG_TYPE_INFO)
#define LOG_ERROR(msg) XHelper::x_log(__FUNCTION__,__FILE__,__LINE__,(msg),XLOG_TYPE_ERROR)
#define LOG_FATAL(msg) XHelper::x_log(__FUNCTION__,__FILE__,__LINE__,(msg),XLOG_TYPE_FATAL)

#define TRY_CATCH(x,...) do{\
    try{x;}catch(const std::exception &e){\
    std::cerr << e.what() << "\n" << std::flush;\
    __VA_ARGS__;}\
}while(false)

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
        const auto _p_ {static_cast<const void * const>(x)};\
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
    const auto _p_{x};\
    static_assert(std::is_pointer_v<std::remove_cv_t<decltype(_p_)>>,#x);\
    try{\
        XHelper::check_nullptr(#x,__FILE__,__LINE__,static_cast<const void*>(_p_));\
    }catch(...){\
        __VA_ARGS__;\
        throw;}\
}while(false)

#define IS_NULLPTR(x,...) do{ \
    const auto _p_ {x};           \
    static_assert(std::is_pointer_v<std::remove_cv_t<decltype(_p_)>>,#x); \
    if(XHelper::is_Nullptr_(#x,__FILE__,__LINE__,static_cast<const void*>(_p_))){ \
        __VA_ARGS__;\
    }\
}while(false)

#define IS_SMART_NULLPTR(x,...) do{ \
    const auto &_smart_ptr_{x};\
    const auto _p_{_smart_ptr_.operator->()};\
    static_assert(std::is_pointer_v<std::remove_cv_t<decltype(_p_)>>,#x); \
    if(XHelper::is_Nullptr_(#x,__FILE__,__LINE__,static_cast<const void*>(_p_))){ \
        __VA_ARGS__;\
    }\
}while(false)

/**
 * x需为false,否则忽略
 * @param x
 * @param ... 用于你的处理
 */
#define CHECK_FALSE_(x,...)do{\
    const auto b_{x};\
    if (XHelper::is_false(#x,__FILE__,__LINE__,b_)){\
    __VA_ARGS__;}\
}while(false)

#define CHECK_EXC(x,...)do{ \
    try{x;}catch(const std::exception &e){ \
    __VA_ARGS__;\
    XHelper::check_EXC(#x,__FILE__,__LINE__,e);}\
}while(false)

#define PRINT_ERR_TIPS(msg) do{ \
    XHelper::print_err_tips(__FUNCTION__,__FILE__,__LINE__,(msg));}while(false)

#define GET_STR(args) #args

#define X_DISABLE_COPY(Class) \
    Class(const Class &) = delete;\
    Class &operator=(const Class &) = delete;

#define X_DISABLE_COPY_MOVE(Class) \
    X_DISABLE_COPY(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete;

template<typename F>
class Destroyer final{
public:
    constexpr inline explicit Destroyer(F &&f):
    fn(std::move(f)){}

    constexpr inline void destroy() {
        if (!is_destroy) {
            is_destroy = true;
            fn();
        }
    }

    constexpr inline ~Destroyer() {
        destroy();
    }

private:
    F fn;
    std::atomic_bool is_destroy{};
    X_DISABLE_COPY(Destroyer)
};

template<typename F2>
class XRAII final {

public:
    constexpr inline explicit XRAII(auto &&f1,F2 &&f2):
    m_f2(std::move(f2)){
        f1();
    }

    constexpr inline void destroy(){
        if (!m_is_destroy){
            m_is_destroy = true;
            m_f2();
        }
    }

    constexpr inline ~XRAII(){
        destroy();
    }

private:
    F2 m_f2{};
    std::atomic_bool m_is_destroy{};
    X_DISABLE_COPY(XRAII)
};

#endif
