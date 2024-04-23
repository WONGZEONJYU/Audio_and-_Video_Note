//
// Created by Administrator on 2024/4/20.
//

#include "AudioOutputStream.hpp"
#include "Muxer.h"
#include "AudioEncoder.h"

void AudioOutputStream::Construct(const std::shared_ptr<Muxer> &muxer,
                                const Audio_encoder_params &encoderParams ,
                                const Audio_Resample_Params &audioResampleParams) noexcept(false) {

    m_encoder = new_AudioEncoder(encoderParams);

    m_stream = muxer->create_stream();

    m_encoder->parameters_from_context(m_stream->codecpar);

    auto ar_params{audioResampleParams};

    const auto audioEncoder {std::dynamic_pointer_cast<AudioEncoder>(m_encoder)};

    ar_params.m_dst_ch_layout = audioEncoder->channel_layout();
    ar_params.m_dst_sample_fmt = audioEncoder->sample_fmt();
    ar_params.m_dst_sample_rate = audioEncoder->sample_rate();

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

AudioOutputStream_sp_type new_AudioOutputStream(const std::shared_ptr<Muxer>& muxer,
                                                const Audio_encoder_params& audioEncoderParams,
                                                const Audio_Resample_Params &audioResampleParams) noexcept(false)
{
    return AudioOutputStream::create(muxer,audioEncoderParams,audioResampleParams);
}
