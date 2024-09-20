
#include "xhelper.hpp"

#ifdef HAVE_FFMPEG
extern "C"{
#include <libavutil/error.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>
#include <libavfilter/avfilter.h>
}
#endif

#ifdef HAVE_OPENGL
#include <SDL_opengl.h>
#endif

#ifdef HAVE_SDL2
#include <SDL.h>
#endif

#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>

using namespace std;

namespace XHelper {
#ifdef HAVE_FFMPEG

    string av_get_err(const int& error_num) noexcept(true) {
        constexpr auto ERROR_STRING_SIZE {1024};
        char err_buf[ERROR_STRING_SIZE]{};
        av_strerror(error_num, err_buf, std::size(err_buf));
        return {err_buf};
    }

    void av_filter_graph_dump(AVFilterGraph * Graph,const string & filename) noexcept(false) {

        ofstream graphFile(filename,std::ios::trunc);

        if (!graphFile){
            throw std::runtime_error("open out_graphFile failed\n");
        }

        auto graph_src {avfilter_graph_dump(Graph, nullptr)};

        if (!graph_src){
            throw std::runtime_error("avfilter_graph_dump alloc string failed\n");
        }

        graphFile << graph_src;

        av_freep(&graph_src);
    }

    void log_packet(const AVFormatContext &fmt_ctx, const AVPacket &pkt) noexcept(true) {

        const auto time_base{&fmt_ctx.streams[pkt.stream_index]->time_base};
        char str_temp[AV_TS_MAX_STRING_SIZE]{};

        const auto fill_str_temp{[&]{
            fill_n(str_temp,AV_TS_MAX_STRING_SIZE,0);
        }};

        cerr << "pts: " << av_ts_make_string(str_temp,pkt.pts) <<
                " , pts_time: " << (fill_str_temp(),av_ts_make_time_string(str_temp,pkt.pts, time_base)) <<
                " , dts: " << (fill_str_temp(),av_ts_make_string(str_temp,pkt.dts)) <<
                " , dts_time: " << (fill_str_temp(),av_ts_make_time_string(str_temp,pkt.dts, time_base)) <<
                " , duration: " << (fill_str_temp(),av_ts_make_string(str_temp,pkt.duration)) <<
                " , duration_time: " << (fill_str_temp(),av_ts_make_time_string(str_temp,pkt.duration, time_base)) <<
                " , stream_index " << pkt.stream_index << "\n" ;
    }

    void check_ff_func(const string &func,const string &file,
                       const int &line,const int &err_code) noexcept(false) {
        if (err_code < 0){
            stringstream err_msg;
            err_msg << "ffmpeg error:" << err_code <<
                        " at " << file << " : " << line <<
                        " -for " << func << " wrong reason: " <<
                        av_get_err(err_code) << "\n";

            throw runtime_error(err_msg.str());
        }
    }

    void ff_err_out(const string &func,const string &file,
                    const int &line,const int &err_code) noexcept(true){
        if (err_code < 0){
            stringstream err_msg;
            err_msg << "ffmpeg error: " << err_code <<
                    " at " << file << " : " << line <<
                    " -for " << func << " wrong reason: " <<
                    av_get_err(err_code) << "\n";
            cerr << err_msg.str();
        }
    }

    string channel_layout_describe(const AVChannelLayout &ch) noexcept(true) {
        char describe[1024]{};
        av_channel_layout_describe(&ch,describe, size(describe));
        return describe;
    }

#endif

#ifdef HAVE_OPENGL
    void checkOpenGLError(const string &stmt, const string &fname,const int &line) noexcept(true){

        const auto err{glGetError()};
        if(GL_NO_ERROR != err) {
            cerr << "OpenGL error :" << err << " at " << fname << ":" << line << " - for " << stmt;
        }
    }
#endif

#ifdef HAVE_SDL2
    void sdl2_err_out(const std::string &func,const std::string &file,
                      const int &line) noexcept(true){
        cerr << "SDL2 error: " <<  SDL_GetError() << " at " << file << " : " << line << " - for " << func << "\n";
    }
#endif

    void check_nullptr(const string &func,const string &file,
                       const int &line,const void *p) noexcept(false){
        if (!p){
            stringstream err_msg;
            err_msg << "error: at " << file << " : " << line <<
                    " -for " << func << " return is nullptr\n";
            throw runtime_error(err_msg.str());
        }
    }

    void check_EXC(const string &func,const string &file,
                   const int &line,const exception &e) noexcept(false){
        stringstream err_msg;
        err_msg << "error: at " << file << " : " << line <<
                " -for " << func << " wrong reason: " << e.what() << '\n';
        throw runtime_error(err_msg.str());
    }

    error_code make_error_code_helper(const int &errcode) noexcept(true) {
        return std::make_error_code(static_cast<std::errc>(errcode));
    }

    void print_err_tips(const string &func,const string &file,
                        const int &line,const string &msg) noexcept(true){
        stringstream err_msg;
        err_msg << "error: at" << file << " : " << line <<
                " -for " << func << " wrong reason: " << msg << '\n';
        cerr << err_msg.str();
    }

    int64_t Get_time_ms() {
        const auto now_{std::chrono::high_resolution_clock::now()};
        const auto now_ms{std::chrono::time_point_cast<std::chrono::milliseconds>(now_)};
        return now_ms.time_since_epoch().count();
    }

    void MSleep(const uint64_t &ms) {
        const auto begin{Get_time_ms()};
        auto ms_{ms};
        while (ms_--) {
            std::this_thread::sleep_for(1ms);
            const auto now{Get_time_ms()};
            if (now - begin >= ms){
                return;
            }
        }
    }
}

static void yuvMirror(uint8_t* yuv,const int &w, const int &h) {

    constexpr auto swap_{[](uint8_t &a, uint8_t &b){
        a ^= b;
        b ^= a;
        a ^= b;
#if 0
        //等价于
        uint8_t temp{a};
        a = b;
        b = temp;
#endif
    }};

    for (int i {};i < h;++i) {
        auto a{i * w},b{(i + 1) * w - 1};
        while (a < b){
            swap_(yuv[a], yuv[b]);
            a++;b--;
        }
    }

    const auto uStartPos{w * h};
    for (int i{};i < (h / 2);++i) {
        auto a{i * w / 2},b{(i + 1) * w / 2 - 1};
        while (a < b){
            swap_(yuv[a+ uStartPos], yuv[b+ uStartPos]);
            a++;b--;
        }
    }

    const auto vStartPos{w * h / 4 * 5};
    for (int i{};i < (h / 2);++i) {
        auto a{i * w / 2},b{(i + 1) * w / 2 - 1};
        while (a < b){
            swap_(yuv[a+ vStartPos], yuv[b+ vStartPos]);
            a++;b--;
        }
    }
}
