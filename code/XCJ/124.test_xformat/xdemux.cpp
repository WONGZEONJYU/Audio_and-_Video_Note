//
// Created by wong on 2024/9/21.
//

extern "C"{
#include <libavformat/avformat.h>
}
#include "xdemux.hpp"
#include "xavpacket.hpp"

#define check_ctx \
std::unique_lock locker(m_mux); \
if(!m_fmt_ctx) { \
PRINT_ERR_TIPS(GET_STR(ctx is empty)); \
return {};}

AVFormatContext *XDemux::Open(const std::string &url) {

    AVFormatContext *c{};
    FF_ERR_OUT(avformat_open_input(&c,url.c_str(), nullptr, nullptr),return {});
    FF_ERR_OUT(avformat_find_stream_info(c, nullptr),return {});
    av_dump_format(c,0,url.c_str(),0);
    return c;
}

bool XDemux::Read(XAVPacket *packet) {

    if (!packet){
        PRINT_ERR_TIPS(GET_STR(packet is empty));
        return {};
    }
    check_ctx;
    FF_ERR_OUT(av_read_frame(m_fmt_ctx,packet),return {});
    return true;
}
