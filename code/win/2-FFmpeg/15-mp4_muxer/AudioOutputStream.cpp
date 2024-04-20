//
// Created by Administrator on 2024/4/20.
//

#include "AudioOutputStream.hpp"

void AudioOutputStream::Construct(const std::shared_ptr<Muxer> &, const Audio_encoder_params &) noexcept(false) {

}

AudioOutputStream_sp_type AudioOutputStream::create(const std::shared_ptr<Muxer> &, const Audio_encoder_params &) {
    return {};
}
