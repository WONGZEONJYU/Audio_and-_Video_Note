//
// Created by wong on 2024/9/22.
//
#define check_ctx \
std::unique_lock locker(m_mux);do{\
if(!m_fmt_ctx) {\
PRINT_ERR_TIPS(GET_STR(ctx is empty));\
return {};}}while(false)

extern "C"{
#include <libavformat/avformat.h>
}
#include "xmux.hpp"
#include "xavpacket.hpp"

AVFormatContext *XMux::Open(const std::string &url) {

    if (url.empty()){
        PRINT_ERR_TIPS(GET_STR(url is empty!));
        return {};
    }

    bool need_free{};
    AVFormatContext *c{};

    Destroyer d([&]{
        if (need_free){
            avformat_free_context(c);
        }
    });

    FF_ERR_OUT(avformat_alloc_output_context2(&c, nullptr, nullptr,url.c_str()),
               return {});

    AVStream *vs,*as;

    TRY_CATCH(CHECK_NULLPTR(vs = avformat_new_stream(c, nullptr)),
              need_free = true;return {});
    vs->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;

    TRY_CATCH(CHECK_NULLPTR(as = avformat_new_stream(c, nullptr)),
              need_free = true;return {});
    as->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;

    FF_ERR_OUT(avio_open(&c->pb,url.c_str(),AVIO_FLAG_WRITE),
               need_free = true;return {});

    return c;
}

bool XMux::WriteHead(){
    check_ctx;
    FF_ERR_OUT(avformat_write_header(m_fmt_ctx, nullptr),return {});
    av_dump_format(m_fmt_ctx,0, m_fmt_ctx->url,1);
    return true;
}

bool XMux::Write(XAVPacket &packet){
    check_ctx;
    FF_ERR_OUT(av_interleaved_write_frame(m_fmt_ctx,&packet),return {});
    return true;
}

bool XMux::WriteEnd(){
    check_ctx;
    av_interleaved_write_frame(m_fmt_ctx, nullptr);
    FF_ERR_OUT(av_write_trailer(m_fmt_ctx),return {});
    return true;
}


