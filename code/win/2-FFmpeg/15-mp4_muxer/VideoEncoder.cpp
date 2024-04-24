//
// Created by Administrator on 2024/4/18.
//

extern "C"{
#include <libavcodec/avcodec.h>
}

#include "VideoEncoder.h"
#include "AVHelper.h"

VideoEncoder::VideoEncoder_sp_type VideoEncoder::create(const Video_Encoder_params & params) noexcept(false) {

    VideoEncoder_sp_type obj;
    try {
        obj = std::move(VideoEncoder_sp_type(new VideoEncoder));
    } catch (const std::bad_alloc &e) {
        throw std::runtime_error("new VideoEncoder failed" + std::string (e.what()) + "\n");
    }

    try {
        obj->Construct(params);
        return obj;
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw std::runtime_error("VideoEncoder Construct failed: " + std::string (e.what()) + "\n");
    }
}

void VideoEncoder::Construct(const Video_Encoder_params &params) noexcept(false) {
    init_codec(params);
}

void VideoEncoder::init_codec(const Video_Encoder_params &params) noexcept(false) {

    auto codec{avcodec_find_encoder(params.m_Codec_ID)};
    if (!codec){
        throw std::runtime_error("AV_CODEC_ID_H264 codec not found\n");
    }

    m_codec_ctx = avcodec_alloc_context3(codec);
    if (!m_codec_ctx){
        throw std::runtime_error("video Codec_Context alloc failed!\n");
    }

    m_codec_ctx->flags  = params.m_flags;
    m_codec_ctx->width = params.m_width;
    m_codec_ctx->height = params.m_height;
    m_codec_ctx->pix_fmt = params.m_pix_fmt;
    m_codec_ctx->time_base = params.m_time_base;
    m_codec_ctx->bit_rate = params.m_bit_rate;
    m_codec_ctx->gop_size = params.m_gop_size;
    m_codec_ctx->max_b_frames = params.max_b_frames;
    m_codec_ctx->framerate = params.m_framerate;

    const auto ret{avcodec_open2(m_codec_ctx, nullptr, nullptr)};
    if (ret < 0){
        throw std::runtime_error("video codec open failed: " + AVHelper::av_get_err(ret) + "\n");
    }

    std::cerr << "init video encoder success\n";
}


VideoEncoder_sp_type new_VideoEncoder(const Video_Encoder_params &params) noexcept(false)
{
    return VideoEncoder::create(params);
}
