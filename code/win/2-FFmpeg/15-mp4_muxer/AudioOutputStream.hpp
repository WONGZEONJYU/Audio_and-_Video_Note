//
// Created by Administrator on 2024/4/20.
//

#ifndef INC_15_MP4_MUXER_AUDIOOUTPUTSTREAM_HPP
#define INC_15_MP4_MUXER_AUDIOOUTPUTSTREAM_HPP

#include "OutputStreamAbstract.h"
#include "AudioEncoder.h"

class Muxer;

class AudioOutputStream final : public OutputStreamAbstract{

    explicit AudioOutputStream() = default;
    void Construct(const std::shared_ptr<Muxer>&,
                   const Audio_encoder_params&) noexcept(false);

public:
    using AudioOutputStream_sp_type = std::shared_ptr<AudioOutputStream>;
    static AudioOutputStream_sp_type create(const std::shared_ptr<Muxer>&,
                                            const Audio_encoder_params&);

private:
    AudioEncoder_sp_type m_audio_encoder;
};

using AudioOutputStream_sp_type = typename AudioOutputStream::AudioOutputStream_sp_type;

#endif //INC_15_MP4_MUXER_AUDIOOUTPUTSTREAM_HPP
