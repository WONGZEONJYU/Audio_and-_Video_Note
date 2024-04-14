#ifndef INC_14_MUXING_FLV_AUDIOOUTPUTSTREAM_H
#define INC_14_MUXING_FLV_AUDIOOUTPUTSTREAM_H

#include "OutputStreamAbstract.h"
#include "SwrContext_t.h"
#include <memory>

struct AVCodecContext;

class AudioOutputStream final : public OutputStreamAbstract{

    static inline constexpr auto STREAM_SAMPLE_RATE{44100};
    static inline constexpr auto STREAM_BIT_RATE{64000};
    static inline constexpr auto STREAM_SAMPLE_FMT{AV_SAMPLE_FMT_S16};

    using AudioOutputStream_sp_type = std::shared_ptr<AudioOutputStream>;

    bool construct() noexcept;
    explicit AudioOutputStream(AVFormatContext &fmt_ctx) noexcept;
    void get_one_audio_frame_data();
    [[nodiscard]]bool get_one_audio_frame() noexcept;
    AVFrame *alloc_audio_frame(const AVSampleFormat &) noexcept;
    void convert() noexcept(false);
    void config_codec_params() override ;
    void config_PCM_params();
    bool open_audio() noexcept;
    bool swr_init() noexcept;

public:
    static std::shared_ptr<OutputStreamAbstract> create(AVFormatContext&) noexcept(false);
    ~AudioOutputStream() override = default;
    bool write_frame() noexcept(false) override;

private:
    double m_t{}, m_tincr{}, m_tincr2{};
    int64_t m_samples_count{};
    SwrContext_t::SwrContext_sp_t m_swr;
};

#endif
