//
// Created by wong on 2024/9/21.
//

#define check_ctx \
std::unique_lock locker(m_mux);do{\
if(!m_fmt_ctx) { \
PRINT_ERR_TIPS(GET_STR(ctx is empty)); \
return {};}}while(false)

extern "C"{
#include <libavformat/avformat.h>
}
#include "xdemux.hpp"
#include "xavpacket.hpp"

AVFormatContext *XDemux::Open(const std::string &url) {

    if (url.empty()){
        PRINT_ERR_TIPS(GET_STR(url is empty!));
        return {};
    }

    AVFormatContext *c{};
    AVDictionary *opts{};
    bool need_free{};

    Destroyer d([&]{
        av_dict_free(&opts);
        if (need_free){
            avformat_close_input(&c);
        }
    });

    //FF_ERR_OUT(av_dict_set(&opts, GET_STR(rtsp_transport), GET_STR(tcp),0),return {});
    FF_ERR_OUT(av_dict_set(&opts, GET_STR(timeout),GET_STR(1000000),0),return {});
    FF_ERR_OUT(avformat_open_input(&c,url.c_str(), nullptr, &opts),return {});
    FF_ERR_OUT(avformat_find_stream_info(c, nullptr),need_free = true;return {});
    av_dump_format(c,0,url.c_str(),0);
    return c;
}

bool XDemux::Read(XAVPacket &packet) {

    check_ctx;
    FF_ERR_OUT(av_read_frame(m_fmt_ctx,&packet),return {});
    m_last_time = XHelper::Get_time_ms();
    return true;
}

bool XDemux::Seek(const int64_t &pts,const int &stream_index) {

    if (stream_index < 0 || pts <= 0) {
        PRINT_ERR_TIPS(GET_STR(params error!));
        return {};
    }
    check_ctx;
    FF_ERR_OUT(av_seek_frame(m_fmt_ctx,stream_index,pts,
                             AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD),return {});
    return true;
}
