//
// Created by Administrator on 2024/4/19.
//

#ifndef INC_15_MP4_MUXER_AUDIOENCODER_H
#define INC_15_MP4_MUXER_AUDIOENCODER_H

#include "EncoderAbstract.h"

struct Audio_encoder_params{
    constexpr explicit Audio_encoder_params(const int &bit_rate = 128*1024,
                                   const int &sample_rate = 44100,
                                   const AVSampleFormat & = AV_SAMPLE_FMT_S16,
                                   const AVChannelLayout & = AV_CHANNEL_LAYOUT_STEREO,
                                   const int &flags = AV_CODEC_FLAG_GLOBAL_HEADER,
                                   const AVCodecID &  = AV_CODEC_ID_AAC);

    const int m_sample_rate{},m_flags{};
    const int64_t m_bit_rate{};
    const AVSampleFormat m_sample_fmt{};
    const AVChannelLayout m_ch_layout{};
    const AVCodecID m_codec_id{};
};

struct AVCodecContext;

class AudioEncoder final : public EncoderAbstract{

    explicit AudioEncoder() = default;
    void Construct(const Audio_encoder_params&) noexcept(false);
    void init_codec(const Audio_encoder_params&) noexcept(false);
public:
    using AudioEncoder_sp_type = std::shared_ptr<AudioEncoder>;
    static AudioEncoder_sp_type create(const Audio_encoder_params&);
};

using AudioEncoder_sp_type = AudioEncoder::AudioEncoder_sp_type;

#endif
