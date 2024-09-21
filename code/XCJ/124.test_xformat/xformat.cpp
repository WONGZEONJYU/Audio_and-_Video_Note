//
// Created by wong on 2024/9/21.
//

extern "C"{
#include <libavformat/avformat.h>
}
#include "xformat.hpp"

#define check_ctx \
std::unique_lock locker(m_mux); \
if(!m_fmt_ctx) { \
PRINT_ERR_TIPS(GET_STR(ctx is empty)); \
return {};}

XFormat::XFormat(){
    m_video_timebase.store({1,25});
    m_audio_timebase.store({1,44100});
}

void XFormat::destroy() {
    if (m_fmt_ctx){
        if (m_fmt_ctx->oformat){ //输出上下文
            if (m_fmt_ctx->pb){
                avio_closep(&m_fmt_ctx->pb);
            }
            avformat_free_context(m_fmt_ctx);
        } else if (m_fmt_ctx->iformat){ //输入上下文
            avformat_close_input(&m_fmt_ctx);
        } else{
            avformat_free_context(m_fmt_ctx);
        }
    }
}

void XFormat::set_fmt_ctx(AVFormatContext *ctx) {
    std::unique_lock locker(m_mux);
    destroy();
    m_fmt_ctx = ctx;
    if (m_fmt_ctx) {
        for (int i {}; i < m_fmt_ctx->nb_streams; ++i) {
            const auto stream{m_fmt_ctx->streams[i]};
            const auto codecpar{stream->codecpar};
            if (AVMEDIA_TYPE_VIDEO == codecpar->codec_type){
                m_video_index = i;
                m_video_timebase.store({stream->time_base.num,stream->time_base.den});
            } else if (AVMEDIA_TYPE_AUDIO == codecpar->codec_type){
                m_audio_index = i;
                m_audio_timebase.store({stream->time_base.num,stream->time_base.den});
            } else{}
        }
    }
}

bool XFormat::CopyParm(const int &stream_index,AVCodecParameters *dst) {
    if (!dst){
        PRINT_ERR_TIPS(GET_STR(dst is empty));
        return {};
    }
    check_ctx
    if (stream_index < 0 || stream_index >= m_fmt_ctx->nb_streams){
        PRINT_ERR_TIPS(GET_STR(stream_index not in range));
        return {};
    }

    FF_ERR_OUT(avcodec_parameters_copy(dst,m_fmt_ctx->streams[stream_index]->codecpar),return {});
    return true;
}

XFormat::~XFormat() {
    std::unique_lock locker(m_mux);
    destroy();
}
