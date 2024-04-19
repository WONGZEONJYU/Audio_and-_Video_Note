//
// Created by Administrator on 2024/4/15.
//
extern "C" {
#include <libavformat/avformat.h>
}

#include "Muxer.h"
#include "AVHelper.h"

Muxer::Muxer_sp_type Muxer::create(std::string &&url) {

    Muxer_sp_type obj;
    try {
        obj = Muxer_sp_type(new Muxer(std::move(url)));
    } catch (const std::bad_alloc &e) {
        throw std::runtime_error("new Muxer failed: " + std::string(e.what()) + "\n");
    }

    try {
        obj->Construct();
        return obj;
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw std::runtime_error("Muxer Construct failed: " + std::string(e.what()) + "\n");
    }
}

Muxer::Muxer(std::string &&url):m_url(std::move(url)) {

}

void Muxer::Construct() {

    Alloc_FormatContext();
    open();
}

void Muxer::Alloc_FormatContext() noexcept(false){
    const auto ret {avformat_alloc_output_context2(&m_fmt_ctx, nullptr, nullptr,m_url.c_str())};
    if (ret < 0){
        throw std::runtime_error("avformat_alloc_output_context2 failed: " + AVHelper::av_get_err(ret) + "\n");
    }
}

void Muxer::open() noexcept(false) {

    if (!(m_fmt_ctx->flags & AVFMT_NOFILE)){
        const auto ret{avio_open(&m_fmt_ctx->pb,m_url.c_str(),AVIO_FLAG_WRITE)};
        if (ret < 0){
            throw std::runtime_error("Could not open " + m_url + " : " + AVHelper::av_get_err(ret) + "\n");
        }
    }
}

void Muxer::Send_header() {
   const auto ret{avformat_write_header(m_fmt_ctx, nullptr)};
    if (ret < 0){
        throw std::runtime_error("avformat_write_header failed: " + AVHelper::av_get_err(ret) + "\n");
    }
}

void Muxer::Send_packet(const ShareAVPacket::ShareAVPacket_sp_type& pkt,const AVRational &src,const AVRational& dst) noexcept(false) {

    // 时间基转换
    pkt->m_packet->pts = av_rescale_q(pkt->m_packet->pts,src,dst);
    pkt->m_packet->dts = av_rescale_q(pkt->m_packet->dts,src,dst);
    pkt->m_packet->duration = av_rescale_q(pkt->m_packet->duration,src,dst);

    const auto ret{av_interleaved_write_frame(m_fmt_ctx,pkt->m_packet)};

    if (ret < 0){
        throw std::runtime_error("av_interleaved_write_frame failed: " + AVHelper::av_get_err(ret) + "\n");
    }
}

void Muxer::Send_trailer() noexcept(false) {
    const auto ret{av_write_trailer(m_fmt_ctx)};
    if (ret < 0){
        throw std::runtime_error("av_write_trailer failed: " + AVHelper::av_get_err(ret ) + "\n");
    }
}

AVStream *Muxer::create_stream() noexcept(false)
{
    auto stream{avformat_new_stream(m_fmt_ctx, nullptr)};
    if (!stream){
        throw std::runtime_error("stream is empty\n");
    }

    return stream;
}

void Muxer::DeConstruct() noexcept {
    if (m_fmt_ctx){
        if (!(m_fmt_ctx->flags & AVFMT_NOFILE)) {
            avio_closep(&m_fmt_ctx->pb);
        }
        avformat_close_input(&m_fmt_ctx);
    }
}

Muxer::~Muxer() {
    DeConstruct();
}
