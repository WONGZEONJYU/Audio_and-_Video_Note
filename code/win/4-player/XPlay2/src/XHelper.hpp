
#ifndef XHELPER_H
#define XHELPER_H

#include <string>
#include <atomic>
#include <system_error>

#ifdef HAVE_FFMPEG
struct AVFormatContext;
struct AVPacket;
struct AVFilterGraph;
struct AVChannelLayout;
#endif

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

    void check_nullptr(const std::string &func,const std::string &file,
                       const int &line,const void *p) noexcept(false);

    void check_EXC(const std::string &func,const std::string &file,
                   const int &line,const std::exception &e) noexcept(false);

    std::error_code make_error_code_helper(const int &errcode) noexcept(true);
}

#ifdef HAVE_FFMPEG
    #define FF_CHECK_ERR(x,...) do{\
    const auto _err_code_{x};\
    if(_err_code_ < 0){\
        __VA_ARGS__;\
        XHelper::check_ff_func(#x,__FILE__,__LINE__,_err_code_);\
    }}while(false)

    #define FF_ERR_OUT(x) do{ \
        XHelper::ff_err_out(#x,__FILE__,__LINE__,x);\
    }while(false)

#endif

#ifdef HAVE_OPENGL
#define GL_CHECK(x) do { \
        x;\
        XHelper::checkOpenGLError(#x, __FILE__, __LINE__); \
    } while (false)
#endif

#define CHECK_NULLPTR(x) do{ \
    const auto _p_ {x};\
        XHelper::check_nullptr(#x,__FILE__,__LINE__,static_cast<const void*>(_p_));\
}while(false)

#define CHECK_EXC(x,...)do{ \
    try{x;}catch(const std::exception &e){ \
    __VA_ARGS__;\
    XHelper::check_EXC(#x,__FILE__,__LINE__,e);}\
}while(false)

#define GET_STR(args) #args

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
    inline explicit XRAII(F1 &&f1,F2 &&f2) : m_f2(std::move(f2)){
        f1();
    }

    inline void destroy(){
        if (!m_is_destroy){
            m_is_destroy = true;
            m_f2();
        }
    }

    inline ~XRAII(){
        destroy();
    }

private:
    F2 m_f2;
    std::atomic_bool m_is_destroy{};
};

// 辅助宏，用于计数
#define GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,NAME,...) NAME
#define VA_SIZE(...) GET_MACRO(__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#define VA_CALL(MACRO, ...) MACRO(VA_SIZE(__VA_ARGS__), __VA_ARGS__)

// 定义两个变参宏，根据参数数量调用不同的实现
#define MY_MACRO_IMPL_2(count, fixed, ...) XRAII r(fixed, __VA_ARGS__)
#define MY_MACRO_IMPL_3(count, fixed, ...) XRAII r(fixed, __VA_ARGS__)

// 选择适当的实现
#define MY_ADVANCED_MACRO(...) VA_CALL(MY_MACRO_SELECT, __VA_ARGS__)
#define MY_MACRO_SELECT(count, ...) MY_MACRO_IMPL_##count(__VA_ARGS__)

#endif