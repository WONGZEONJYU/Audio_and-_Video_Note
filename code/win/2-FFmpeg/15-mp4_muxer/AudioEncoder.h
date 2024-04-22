//
// Created by Administrator on 2024/4/19.
//

#ifndef INC_15_MP4_MUXER_AUDIOENCODER_H
#define INC_15_MP4_MUXER_AUDIOENCODER_H

#include "EncoderAbstract.h"

struct Audio_encoder_params{
    constexpr explicit Audio_encoder_params(const int &bit_rate = 128*1024,
                                   const int &sample_rate = 44100,
                                   const AVSampleFormat &sample_fmt = AV_SAMPLE_FMT_FLTP,
                                   const AVChannelLayout &ch_layout = AV_CHANNEL_LAYOUT_STEREO,
                                   const int &flags = AV_CODEC_FLAG_GLOBAL_HEADER,
                                   const AVCodecID &codec_id  = AV_CODEC_ID_AAC) noexcept(true):
            m_sample_rate(sample_rate),m_flags(flags),
            m_bit_rate(bit_rate),m_sample_fmt{sample_fmt},
            m_ch_layout{ch_layout},m_codec_id(codec_id){}

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

    [[nodiscard]] auto sample_rate() const noexcept(true){
        return m_codec_ctx->sample_rate;
    }

    [[nodiscard]] auto sample_fmt() const noexcept(true){
        return m_codec_ctx->sample_fmt;
    }

    [[nodiscard]] auto channel_layout() const noexcept(true){
        return m_codec_ctx->ch_layout;
    }

    [[nodiscard]] auto frame_size() const noexcept(true){
        return m_codec_ctx->frame_size;
    }
};

using AudioEncoder_sp_type = AudioEncoder::AudioEncoder_sp_type;

#endif
