//
// Created by Administrator on 2024/4/19.
//

extern "C"{
#include <libavcodec/avcodec.h>
}

#include "AudioEncoder.h"
#include "AVHelper.h"

void AudioEncoder::Construct(const Audio_encoder_params& params) noexcept(false)
{
    init_codec(params);
}

AudioEncoder_sp_type AudioEncoder::create(const Audio_encoder_params &params) noexcept(false){

    AudioEncoder_sp_type obj;

    try {
        obj = std::move(AudioEncoder_sp_type(new AudioEncoder));
    }catch (const std::bad_alloc &e){
        throw std::runtime_error("new AudioEncoder failed: " + std::string (e.what()) + "\n");
    }

    try {
        obj->Construct(params);
        return obj;
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw std::runtime_error("AudioEncoder Construct failed: " + std::string (e.what()) + "\n");
    }
}

void AudioEncoder::init_codec(const Audio_encoder_params &params) noexcept(false) {

    auto codec{avcodec_find_encoder(params.m_codec_id)};
    if(!codec){
        throw std::runtime_error("audio encoder not found\n");
    }

    m_codec_ctx = avcodec_alloc_context3(codec);
    if (!m_codec_ctx){
        throw std::runtime_error("alloc audio codec context error\n");
    }

    m_codec_ctx->flags |= params.m_flags;
    m_codec_ctx->bit_rate = params.m_bit_rate;
    m_codec_ctx->ch_layout = params.m_ch_layout;
    m_codec_ctx->sample_fmt = params.m_sample_fmt;
    m_codec_ctx->sample_rate = params.m_sample_rate;

    const auto ret {avcodec_open2(m_codec_ctx, nullptr, nullptr)};
    if (ret < 0){
        throw std::runtime_error("audio ");
    }

    std::cerr << "init audio encoder success\n";
}

void AudioEncoder::encode(const ShareAVFrame_sp_type &frame,
            const int &stream_index,
            const long long int &pts,
            const AVRational &time_base,
            vector_type &packets) const noexcept(false)
{
    encode("audio",frame,stream_index,pts,time_base,packets);
}

AudioEncoder_sp_type new_AudioEncoder(const Audio_encoder_params &params) noexcept(false)
{
    return AudioEncoder::create(params);
}
