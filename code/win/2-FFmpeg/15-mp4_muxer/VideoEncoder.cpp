//
// Created by Administrator on 2024/4/18.
//

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
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
    init_frame();
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

void VideoEncoder::init_frame() noexcept(false)
{
    m_frame = new_ShareAVFrame();
    m_frame->m_frame->width = m_codec_ctx->width;
    m_frame->m_frame->height = m_codec_ctx->height;
    m_frame->m_frame->format = m_codec_ctx->pix_fmt;
}

void VideoEncoder::image_fill_arrays(const uint8_t* yuv_buffer, const size_t &yuv_buffer_size) noexcept(false)
{
    if (m_frame){
        init_frame();
    }

    const auto ret {av_image_fill_arrays(m_frame->m_frame->data,m_frame->m_frame->linesize,
                                         yuv_buffer,m_codec_ctx->pix_fmt,
                                         m_codec_ctx->width,
                                         m_codec_ctx->height,1)};
    if (ret < 0){
        m_frame.reset();
        throw std::runtime_error("av_image_fill_arrays failed: " + AVHelper::av_get_err(ret) + "\n");
    }else if (yuv_buffer_size != ret){
        m_frame.reset();
        throw std::runtime_error("yuv_buffer_size: " + std::to_string(yuv_buffer_size) +
                                " != ret_size: " + std::to_string(ret) + "\n");
    } else{}
}

void VideoEncoder::encode(const ShareAVFrame_sp_type &frame,
                           const int &stream_index,
                           const int64_t &pts,
                           const AVRational &time_base,
                           vector_type &packets) const noexcept(false)
{
    encode("video",frame,stream_index,pts,time_base,packets);
}

void VideoEncoder::encode(const uint8_t* yuv_buffer,
            const size_t &yuv_size,
            const int &stream_index,
            const int64_t &pts,
            const AVRational &time_base,
            vector_type &packets) noexcept(false)
{
    ShareAVFrame_sp_type frame;
    if (yuv_buffer){
        image_fill_arrays(yuv_buffer,yuv_size);
        frame = m_frame;
    }
    encode(frame,stream_index,pts,time_base,packets);
}

VideoEncoder_sp_type new_VideoEncoder(const Video_Encoder_params &params) noexcept(false)
{
    return VideoEncoder::create(params);
}
