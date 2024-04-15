#ifndef INC_14_MUXING_FLV_AUDIOOUTPUTSTREAM_H
#define INC_14_MUXING_FLV_AUDIOOUTPUTSTREAM_H

#include "OutputStreamAbstract.h"
#include "SwrContext_t.h"

struct AVCodecContext;

class AudioOutputStream final : public OutputStreamAbstract{

    static inline constexpr auto STREAM_SAMPLE_RATE{44100};
    static inline constexpr auto STREAM_BIT_RATE{64000};
    static inline constexpr auto STREAM_SAMPLE_FMT{AV_SAMPLE_FMT_S16};

    void construct() noexcept(false);
    explicit AudioOutputStream(AVFormatContext &fmt_ctx) noexcept(true);
    void get_one_audio_frame_data() noexcept(true);
    [[nodiscard]]bool get_one_audio_frame() noexcept(true);
    AVFrame *alloc_audio_frame(const AVSampleFormat &) noexcept(false);
    void convert() noexcept(false);
    void config_codec_params() noexcept(true) override ;
    void config_PCM_params() noexcept(true);
    void open_audio() noexcept(false);
    void swr_init() noexcept(false);

public:
    static OutputStreamAbstract_sp_type create(AVFormatContext&) noexcept(false);
    ~AudioOutputStream() override = default;
    bool write_frame() noexcept(false) override;

private:
    using AudioOutputStream_sp_type = std::shared_ptr<AudioOutputStream>;
    double m_t{}, m_tincr{}, m_tincr2{};
    int64_t m_samples_count{};
    SwrContext_t::SwrContext_sp_t m_swr;
};

#endif
