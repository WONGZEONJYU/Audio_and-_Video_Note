
#include "XHelper.hpp"

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

#include <iostream>
#include <fstream>
#include <sstream>

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
                        " at " << file << ":" << line <<
                        " -for " << func << " wrong reason: " <<
                        av_get_err(err_code) << "\n";

            throw runtime_error(err_msg.str());
        }
    }

    void ff_err_out(const string &func,const string &file,
                    const int &line,const int &err_code) noexcept(true){
        if (err_code < 0){
            stringstream err_msg;
            err_msg << "ffmpeg error:" << err_code <<
                    " at " << file << ":" << line <<
                    " -for " << func << " wrong reason: " <<
                    av_get_err(err_code) << "\n";
            cerr << err_msg.str();
        }
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

    string channel_layout_describe(const AVChannelLayout &ch) noexcept(true) {
        char describe[1024]{};
        av_channel_layout_describe(&ch,describe, size(describe));
        return describe;
    }

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
}
