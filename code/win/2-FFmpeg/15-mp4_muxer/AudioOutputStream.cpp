//
// Created by Administrator on 2024/4/20.
//

#include "AudioOutputStream.hpp"
#include "Muxer.h"
#include "AudioEncoder.h"

void AudioOutputStream::Construct(const std::shared_ptr<Muxer> &muxer,
        const Audio_encoder_params &params) noexcept(false) {

    m_encoder = AudioEncoder::create(params);

    m_stream = muxer->create_stream();

    m_encoder->parameters_from_context(m_stream->codecpar);
}

AudioOutputStream_sp_type AudioOutputStream::create(const std::shared_ptr<Muxer> &muxer,
                                                    const Audio_encoder_params &params) noexcept(false)
{
    AudioOutputStream_sp_type obj;
    try {
        obj = std::move(AudioOutputStream_sp_type(new AudioOutputStream));
    }catch (const std::bad_alloc &e){
        throw std::runtime_error("new AudioOutputStream failed: " + std::string(e.what()) + "\n");
    }

    try {
        obj->Construct(muxer,params);
        return obj;
    } catch (const std::runtime_error &e) {
        throw std::runtime_error("AudioOutputStream  Construct failed: " + std::string(e.what()) + "\n");
    }
}
