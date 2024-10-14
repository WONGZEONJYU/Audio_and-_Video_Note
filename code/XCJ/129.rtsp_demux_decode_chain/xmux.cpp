//
// Created by wong on 2024/9/22.
//

extern "C"{
#include <libavformat/avformat.h>
}
#include "xmux.hpp"
#include "xavpacket.hpp"
#include "xcodec_parameters.hpp"

AVFormatContext *XMux::Open(const std::string &url,
                            const XCodecParameters_sp &video_parm,
                            const XCodecParameters_sp &audio_parm) {

    if (url.empty()){
        PRINT_ERR_TIPS(GET_STR(url is empty!));
        return {};
    }

    bool need_free{};
    AVFormatContext *c{};

    const Destroyer d([&c,&need_free]{
        if (need_free){
            avformat_free_context(c);
        }
    });

    FF_ERR_OUT(avformat_alloc_output_context2(&c,nullptr, nullptr,url.c_str()),
               return {});

    if (video_parm){
        AVStream *vs{};
        IS_NULLPTR(vs = avformat_new_stream(c, nullptr),need_free = true;return {};);
        video_parm->to_AVCodecParameters(vs->codecpar);
    }

    if (audio_parm){
        AVStream *as{};
        IS_NULLPTR(as = avformat_new_stream(c, nullptr),need_free = true;return {});
        audio_parm->to_AVCodecParameters(as->codecpar);
    }

    FF_ERR_OUT(avio_open(&c->pb,url.c_str(),AVIO_FLAG_WRITE),
               need_free = true;return {});

    av_dump_format(c,0,url.c_str(),1);
    return c;
}

void XMux::set_video_time_base(const AVRational *tb){
    std::unique_lock locker(m_mux_);
    if (!m_src_video_time_base_){
        m_src_video_time_base_ = new AVRational{};
    }
    *m_src_video_time_base_ = *tb;
}

void XMux::set_audio_time_base(const AVRational *tb){
    std::unique_lock locker(m_mux_);
    if (!m_src_audio_time_base_){
        m_src_audio_time_base_ = new AVRational{};
    }
    *m_src_audio_time_base_ = *tb;
}

bool XMux::WriteHead(){
    check_fmt_ctx();
    FF_ERR_OUT(avformat_write_header(m_fmt_ctx_, nullptr),return {});
    av_dump_format(m_fmt_ctx_,0, m_fmt_ctx_->url,1);
    return true;
}

bool XMux::Write(XAVPacket &packet){
    check_fmt_ctx();
    FF_ERR_OUT(av_interleaved_write_frame(m_fmt_ctx_,&packet),return {});
    return true;
}

bool XMux::WriteEnd(){
    check_fmt_ctx();
    av_interleaved_write_frame(m_fmt_ctx_, nullptr);
    FF_ERR_OUT(av_write_trailer(m_fmt_ctx_),return {});
    return true;
}

void XMux::destroy(){
    std::unique_lock locker(m_mux_);
    delete m_src_video_time_base_;
    m_src_video_time_base_ = nullptr;
    delete m_src_audio_time_base_;
    m_src_audio_time_base_ = nullptr;
}

XMux::~XMux(){
    destroy();
}
