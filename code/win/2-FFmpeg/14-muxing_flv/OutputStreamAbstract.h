#ifndef INC_14_MUXING_FLV_OUTPUTSTREAMABSTRACT_H
#define INC_14_MUXING_FLV_OUTPUTSTREAMABSTRACT_H

struct AVFormatContext;
struct AVStream;
struct AVPacket;
struct AVCodec;
struct AVCodecContext;
struct AVFrame;
struct AVRational;

#include <memory>

class OutputStreamAbstract {

protected:
    static inline constexpr auto STREAM_DURATION{5.0};
    AVFormatContext &m_fmt_ctx;
    const AVCodec * m_codec{};
    AVCodecContext *m_codec_ctx{};
    AVStream *m_stream{};
    AVFrame *m_frame{},*m_tmp_frame{};
    long long m_next_pts{};

public:
    OutputStreamAbstract(const OutputStreamAbstract&) = delete;
    OutputStreamAbstract& operator=(const OutputStreamAbstract&) = delete;
    [[nodiscard]] virtual bool write_frame() noexcept(false) = 0 ;
    [[nodiscard("nodiscard nex_pts value")]] auto nex_pts() const noexcept(true){return m_next_pts;};
    [[nodiscard("nodiscard time_base value")]] AVRational time_base() const noexcept(true);
protected:
    using OutputStreamAbstract_sp_type = std::shared_ptr<OutputStreamAbstract>;
    virtual void config_codec_params() noexcept(true) = 0;
    explicit OutputStreamAbstract(AVFormatContext&) noexcept;
    virtual ~OutputStreamAbstract() ;
    void write_media_file(AVPacket &) noexcept(false);
    void add_stream(const int&) noexcept(false);
};

#endif
