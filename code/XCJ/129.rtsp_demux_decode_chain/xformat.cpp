//
// Created by wong on 2024/9/21.
//

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/codec_par.h>
}

#include "xformat.hpp"
#include "xavpacket.hpp"
#include "xcodec_parameters.hpp"

void XFormat::destroy_fmt_ctx(){
    if (m_fmt_ctx_){
        if (m_fmt_ctx_->oformat){ //输出上下文
            if (m_fmt_ctx_->pb){
                avio_closep(&m_fmt_ctx_->pb);
            }
            avformat_free_context(m_fmt_ctx_);
        } else if (m_fmt_ctx_->iformat){ //输入上下文
            avformat_close_input(&m_fmt_ctx_);
        } else{
            avformat_free_context(m_fmt_ctx_);
        }
    }
}

void XFormat::set_fmt_ctx(AVFormatContext *ctx) {
    std::unique_lock locker(m_mux_);
    destroy_fmt_ctx();
    m_fmt_ctx_ = ctx;
    if (!m_fmt_ctx_){
        m_is_connected_ = false;
        return;
    }

    m_is_connected_ = true;
    m_last_time_ = XHelper::Get_time_ms();

    if (m_time_out_ms_ > 0){
        const AVIOInterruptCB cb {Time_out_callback, this};
        m_fmt_ctx_->interrupt_callback = cb;
    }

    m_audio_index_ = m_video_index_ = -1;
    m_video_timebase_ = {1,25};
    m_audio_timebase_ = {1,44100};
    for (int i {}; i < m_fmt_ctx_->nb_streams; ++i) {
        const auto stream{m_fmt_ctx_->streams[i]};
        const auto type{stream->codecpar->codec_type};
        if (AVMEDIA_TYPE_VIDEO == type){
            m_video_index_ = i;
            m_video_timebase_.num = stream->time_base.num;
            m_video_timebase_.den = stream->time_base.den;
            m_codec_id_ = stream->codecpar->codec_id;
        } else if (AVMEDIA_TYPE_AUDIO == type){
            m_audio_index_ = i;
            m_audio_timebase_.num = stream->time_base.num;
            m_audio_timebase_.den = stream->time_base.den;
        } else{}
    }
}

bool XFormat::CopyParm(const int &stream_index,AVCodecParameters *dst) {
    if (!dst){
        PRINT_ERR_TIPS(GET_STR(dst is empty));
        return {};
    }
    check_fmt_ctx();
    if (stream_index < 0 || stream_index >= m_fmt_ctx_->nb_streams){
        PRINT_ERR_TIPS(GET_STR(stream_index not in range));
        return {};
    }

    FF_ERR_OUT(avcodec_parameters_copy(dst,m_fmt_ctx_->streams[stream_index]->codecpar),return {});
    return true;
}

bool XFormat::CopyParm(const int &stream_index,AVCodecContext *dst){

    if (!dst) {
        PRINT_ERR_TIPS(GET_STR(dst is empty));
        return {};
    }
    check_fmt_ctx();
    if (stream_index < 0 || stream_index >= m_fmt_ctx_->nb_streams){
        PRINT_ERR_TIPS(GET_STR(stream_index not in range));
        return {};
    }
    FF_ERR_OUT(avcodec_parameters_to_context(dst,m_fmt_ctx_->streams[stream_index]->codecpar),return {});
    return true;
}

XCodecParameters_sp XFormat::CopyVideoParm() const{
    check_fmt_ctx();

    const auto index{m_video_index_.load()};
    if (index < 0){
        PRINT_ERR_TIPS(GET_STR(no video));
        return {};
    }

    const auto video_st{m_fmt_ctx_->streams[index]};
    return new_XCodecParameters(video_st->codecpar,video_st->time_base);
}

XCodecParameters_sp XFormat::CopyAudioParm() const{
    check_fmt_ctx();
    const auto index{m_audio_index_.load()};
    if (index < 0){
        PRINT_ERR_TIPS(GET_STR(no audio));
        return {};
    }

    const auto audio_st{m_fmt_ctx_->streams[index]};
    return new_XCodecParameters(audio_st->codecpar,audio_st->time_base);
}

bool XFormat::RescaleTime(XAVPacket &packet, const int64_t &offset_pts, const XRational &time_base) {

    check_fmt_ctx();
    const auto out_stream{m_fmt_ctx_->streams[packet.stream_index]};
    const AVRational in_time_base{time_base.num,time_base.den};
    packet.pts = av_rescale_q_rnd(packet.pts - offset_pts,in_time_base,out_stream->time_base,
                                  static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));

    packet.dts = av_rescale_q_rnd(packet.dts - offset_pts,in_time_base,out_stream->time_base,
                                  static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));

    packet.duration = av_rescale_q(packet.duration,in_time_base,out_stream->time_base);
    packet.pos = -1;
    return true;
}

void XFormat::destroy() {
    std::unique_lock locker(m_mux_);
    destroy_fmt_ctx();
}

XFormat::~XFormat() {
    destroy();
}

int XFormat::Time_out_callback(void *const arg) {
    const auto this_{static_cast<XFormat*>(arg)};
    const auto b{this_->IsTimeout()};
    //std::cerr << __func__  << " : " << std::boolalpha << b << "\n";
    return b;
}

bool XFormat::set_timeout_ms(const uint64_t &ms) {
    m_time_out_ms_ = ms;
    check_fmt_ctx();
    m_fmt_ctx_->interrupt_callback = {Time_out_callback,this};
    return true;
}

bool XFormat::IsTimeout() {
    const auto curr_time{XHelper::Get_time_ms()};
    if (curr_time - m_last_time_ > m_time_out_ms_) {
        m_is_connected_ = false;
        m_last_time_ = curr_time;
        return true;
    }
    return {};
}
