//
// Created by Administrator on 2024/4/20.
//

#include "AudioOutputStream.hpp"
#include "Muxer.h"
#include "AudioEncoder.h"

void AudioOutputStream::Construct(const std::shared_ptr<Muxer> &muxer,
                                const Audio_encoder_params &encoderParams ,
                                const Audio_Resample_Params &audioResampleParams) noexcept(false)
{
    m_encoder = new_AudioEncoder(encoderParams);
    m_stream = muxer->create_stream();
    m_encoder->parameters_from_context(m_stream->codecpar);
    auto ar_params{audioResampleParams};

    ar_params.m_dst_ch_layout = m_encoder->channel_layout();
    ar_params.m_dst_sample_fmt = m_encoder->sample_fmt();
    ar_params.m_dst_sample_rate = m_encoder->sample_rate();
    m_audioResample = new_Audio_Resample(ar_params);
    muxer->dump_format(m_stream->index);
}

AudioOutputStream_sp_type AudioOutputStream::create(const std::shared_ptr<Muxer> &muxer,
                                                    const Audio_encoder_params &encoderParams ,
                                                    const Audio_Resample_Params &audioResampleParams
                                                    ) noexcept(false)
{
    AudioOutputStream_sp_type obj;
    try {
        obj = std::move(AudioOutputStream_sp_type(new AudioOutputStream));
    }catch (const std::bad_alloc &e){
        throw std::runtime_error("new AudioOutputStream failed: " + std::string(e.what()) + "\n");
    }

    try {
        obj->Construct(muxer,encoderParams,audioResampleParams);
        return obj;
    } catch (const std::runtime_error &e) {
        throw std::runtime_error("AudioOutputStream  Construct failed: " + std::string(e.what()) + "\n");
    }
}

void AudioOutputStream::encoder(const ShareAVFrame_sp_type &frame,
                                const int64_t &pts,
                                const AVRational &time_base,
                                vector_type &packets) const noexcept(false)
{
    m_encoder->encode(frame,m_stream->index,pts,time_base,packets);
}

AudioOutputStream_sp_type new_AudioOutputStream(const std::shared_ptr<Muxer>& muxer,
                                                const Audio_encoder_params& audioEncoderParams,
                                                const Audio_Resample_Params &audioResampleParams) noexcept(false)
{
    return AudioOutputStream::create(muxer,audioEncoderParams,audioResampleParams);
}
