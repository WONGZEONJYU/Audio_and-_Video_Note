#include "AVHelper.h"
#include <iostream>

namespace  AVHelper {

    std::string av_get_err(const int& errnum) {
        constexpr auto ERROR_STRING_SIZE {1024};
        char err_buf[ERROR_STRING_SIZE]{};
        av_strerror(errnum, err_buf, std::size(err_buf));
        return {err_buf};
    }


    void log_packet(const AVFormatContext &fmt_ctx, const AVPacket &pkt) {

        const auto time_base{&fmt_ctx.streams[pkt.stream_index]->time_base};
        char str_temp[AV_TS_MAX_STRING_SIZE]{};

        const auto fill_str_temp{[&]{
            std::fill_n(str_temp,AV_TS_MAX_STRING_SIZE,0);
        }};

        std::cerr << "pts: " << av_ts_make_string(str_temp,pkt.pts) <<
                " , pts_time: " << (fill_str_temp(),av_ts_make_time_string(str_temp,pkt.pts, time_base)) <<
                " , dts: " << (fill_str_temp(),av_ts_make_string(str_temp,pkt.dts)) <<
                " , dts_time: " << (fill_str_temp(),av_ts_make_time_string(str_temp,pkt.dts, time_base)) <<
                " , duration: " << (fill_str_temp(),av_ts_make_string(str_temp,pkt.duration)) <<
                " , duration_time: " << (fill_str_temp(),av_ts_make_time_string(str_temp,pkt.duration, time_base)) <<
                " , stream_index " << pkt.stream_index << "\n" ;
    }

}
