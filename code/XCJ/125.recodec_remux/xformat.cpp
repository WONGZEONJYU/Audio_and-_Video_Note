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
#include "xformat.hpp"
#include "xavpacket.hpp"

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
    if (!m_fmt_ctx){
        return;
    }
    m_audio_index = m_video_index = -1;
    m_video_timebase = {1,25};
    m_audio_timebase = {1,44100};
    for (int i {}; i < m_fmt_ctx->nb_streams; ++i) {
        const auto stream{m_fmt_ctx->streams[i]};
        const auto type{stream->codecpar->codec_type};
        if (AVMEDIA_TYPE_VIDEO == type){
            m_video_index = i;
            m_video_timebase.num = stream->time_base.num;
            m_video_timebase.den = stream->time_base.den;
        } else if (AVMEDIA_TYPE_AUDIO == type){
            m_audio_index = i;
            m_audio_timebase.num = stream->time_base.num;
            m_audio_timebase.den = stream->time_base.den;
        } else{}
    }
}

bool XFormat::CopyParm(const int &stream_index,AVCodecParameters *dst) {
    if (!dst){
        PRINT_ERR_TIPS(GET_STR(dst is empty));
        return {};
    }
    check_ctx;
    if (stream_index < 0 || stream_index >= m_fmt_ctx->nb_streams){
        PRINT_ERR_TIPS(GET_STR(stream_index not in range));
        return {};
    }

    FF_ERR_OUT(avcodec_parameters_copy(dst,m_fmt_ctx->streams[stream_index]->codecpar),return {});
    return true;
}

bool XFormat::RescaleTime(XAVPacket &packet, const int64_t &offset_pts, const XRational &time_base) {

    check_ctx;
    const auto out_stream{m_fmt_ctx->streams[packet.stream_index]};
    const AVRational in_time_base{time_base.num,time_base.den};
    packet.pts = av_rescale_q_rnd(packet.pts - offset_pts,in_time_base,out_stream->time_base,
                                  static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));

    packet.dts = av_rescale_q_rnd(packet.dts - offset_pts,in_time_base,out_stream->time_base,
                                  static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));

    packet.duration = av_rescale_q(packet.duration,in_time_base,out_stream->time_base);
    packet.pos = -1;
    return true;
}

XFormat::~XFormat() {
    std::unique_lock locker(m_mux);
    destroy();
}
