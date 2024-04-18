//
// Created by Administrator on 2024/4/18.
//

extern "C"{
#include <libavcodec/avcodec.h>
}

#include "VideoEncoder.h"

VideoEncoder::VideoEncoder_sp_type VideoEncoder::create() noexcept(false) {

    VideoEncoder_sp_type obj;
    try {
        obj = std::move(VideoEncoder_sp_type(new VideoEncoder));
    } catch (const std::bad_alloc &e) {
        throw std::runtime_error("new VideoEncoder failed" + std::string (e.what()) + "\n");
    }

    try {
        obj->Construct();
        return obj;
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw std::runtime_error("VideoEncoder Construct failed: " + std::string (e.what()) + "\n");
    }
}

void VideoEncoder::Construct() noexcept(false) {

}



void VideoEncoder::init_codec() noexcept(false) {
    auto codec{avcodec_find_encoder(AV_CODEC_ID_H264)};
    if (!codec){
        throw std::runtime_error("codec not found\n");
    }

    m_codec_ctx = avcodec_alloc_context3(codec);
    if (!m_codec_ctx){
        throw std::runtime_error("video Codec_Context alloc failed!\n");
    }

    m_codec_ctx->width;
    m_codec_ctx->height;
    m_codec_ctx->pix_fmt;
    m_codec_ctx->time_base;
    m_codec_ctx->bit_rate;
    m_codec_ctx->gop_size;
}

void VideoEncoder::encode(AVFrame *frame, const int &stream_index, const long long int &pts, const AVRational &time_base,
                     std::vector<AVPacket *> &packets) const noexcept(false)
{


}

AVRational VideoEncoder::time_base() const noexcept(true) {
    return m_codec_ctx->time_base;
}

void VideoEncoder::DeConstruct() noexcept(true) {
    avcodec_free_context(&m_codec_ctx);
}

VideoEncoder::~VideoEncoder() {
    DeConstruct();
}


