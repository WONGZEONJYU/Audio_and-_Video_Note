//
// Created by Administrator on 2024/4/18.
//

extern "C"{
#include <libavutil/imgutils.h>
}

#include "VideoOutputStream.h"
#include "Muxer.h"

void VideoOutputStream::Construct(const Muxer_sp_type& muxer,
                                  const Video_Encoder_params& params) noexcept(false)
{
    m_encoder = VideoEncoder::create(params);
    m_stream = muxer->create_stream();
    m_encoder->parameters_from_context(m_stream->codecpar);
    m_encoder->h264_set_preset(preset_medium);
    m_encoder->h264_set_tune(tune_zerolatency);
    m_encoder->h264_set_profile(profile_high);
    muxer->dump_format(m_stream->index);
}

VideoOutputStream_sp_type VideoOutputStream::create(const Muxer_sp_type& muxer,
                                                   const Video_Encoder_params& params) noexcept(false)
{
    VideoOutputStream_sp_type obj;

    try {
        obj = VideoOutputStream_sp_type(new VideoOutputStream);
    } catch (const std::bad_alloc &e) {
        throw std::runtime_error("new VideoOutputStream failed: " + std::string (e.what()) + "\n");
    }

    try {
        obj->Construct(muxer,params);
        return obj;
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw std::runtime_error("VideoOutputStream Construct failed: "  + std::string (e.what()) + "\n");
    }
}

void VideoOutputStream::encoder(const uint8_t* yuv_buffer,
                                const size_t& yuv_size,
                                const long long &pts,
                                const AVRational& time_base,
                                vector_type& packets) noexcept(false)
{
    m_encoder->encode(yuv_buffer,yuv_size,m_stream->index,pts,time_base,packets);
}

VideoOutputStream_sp_type new_VideoOutputStream(const Muxer_sp_type &muxer,
                                                const Video_Encoder_params &params) noexcept(false)
{
    return VideoOutputStream::create(muxer,params);
}
