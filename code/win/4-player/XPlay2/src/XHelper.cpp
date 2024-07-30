#include "XHelper.h"
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

    void check_ff_func(const std::string &func,const std::string &file,
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
#endif

    std::string channel_layout_describe(const AVChannelLayout &ch) noexcept(true) {
        char describe[1024]{};
        av_channel_layout_describe(&ch,describe, size(describe));
        return describe;
    }

    void check_nullptr(const std::string &func,const std::string &file,
                       const int &line,const void *p) noexcept(false){

        if (!p){
            stringstream err_msg;
            err_msg << "error:" << " at " << file << ":" << line <<
                    " -for " << func << " return is nullptr\n";
            throw runtime_error(err_msg.str());
        }
    }

    void check_EXC(const std::string &func,const std::string &file,
                   const int &line,const exception &e) noexcept(false){
        stringstream err_msg;
        err_msg << "error:" << " at " << file << ":" << line <<
                " -for " << func << " wrong reason: " << e.what() << '\n';
        throw runtime_error(err_msg.str());
    }

    error_code make_error_code_helper(const int &errcode) noexcept(true) {
        return std::make_error_code(static_cast<std::errc>(errcode));
    }
}
