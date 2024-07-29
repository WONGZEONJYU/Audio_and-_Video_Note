
#ifndef AVHELPER_H
#define AVHELPER_H

extern "C"{
#include <libavutil/error.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>
#include <libavfilter/avfilter.h>
}

#include <iostream>
#include <string>
#include <atomic>

namespace AVHelper {

    inline namespace v1 {

        std::string av_get_err(const int&) noexcept(true);

        void log_packet(const AVFormatContext &, const AVPacket &)  noexcept(true);

        void av_filter_graph_dump(AVFilterGraph *,const std::string &) noexcept(false);

        std::error_code make_error_code_helper(const int &errcode) noexcept(true);

        void check_ff_func(const std::string &func,const std::string &file,
                           const int &line,const int &err_code) noexcept(false);

        void check_nullptr(const std::string &func,const std::string &file,
                           const int &line,const void *p) noexcept(false);

        void check_EXC(const std::string &func,const std::string &file,
                       const int &line,const std::exception &e) noexcept(false);

        std::string channel_layout_describe(const AVChannelLayout &) noexcept(true);
    }

    inline namespace v2{

    }
}

template<typename F>
struct Destroyer final{
    Destroyer(const Destroyer&) = delete;
    Destroyer& operator=(const Destroyer&) = delete;
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

#define FF_CHECK_ERR(x) do{ \
const auto _err_code_{x};\
AVHelper::check_ff_func(#x,__FILE__,__LINE__,_err_code_);\
}while(false)

#define CHECK_NULLPTR(x) do{ \
const auto _p_ {x};\
AVHelper::check_nullptr(#x,__FILE__,__LINE__,static_cast<const void*>(_p_));\
}while(false)

#define CHECK_EXC(x)do{ \
try{\
x;\
}catch(const std::exception &e){\
AVHelper::check_EXC(#x,__FILE__,__LINE__,e);\
}}while(false)

#endif
