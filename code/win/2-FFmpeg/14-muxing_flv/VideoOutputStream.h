#ifndef INC_14_MUXING_FLV_VIDEOOUTPUTSTREAM_H
#define INC_14_MUXING_FLV_VIDEOOUTPUTSTREAM_H

#include "OutputStreamAbstract.h"
#include "SwsContext_t.h"
#include <memory>

class VideoOutputStream final : public OutputStreamAbstract {

    static inline constexpr auto STREAM_PIX_FMT{AV_PIX_FMT_YUV420P};
    static inline constexpr auto STREAM_FRAME_RATE{25};

    void fill_yuv_image(AVFrame &pict) noexcept(true);
    AVFrame *alloc_picture() noexcept(false);
    [[nodiscard]]bool get_one_frame() noexcept(false);
    explicit VideoOutputStream(AVFormatContext &) noexcept(true);
    void construct() noexcept(false);
    void config_codec_params() noexcept(true) override;
    void open_video() noexcept(false);
    void sws_init() noexcept(false);

public:
    [[nodiscard]] bool write_frame() noexcept(false) override ;
    ~VideoOutputStream() override = default;
    static OutputStreamAbstract_sp_type create(AVFormatContext &) noexcept(false);

private:
    using VideoOutputStream_sp_type = std::shared_ptr<VideoOutputStream>;
    SwsContext_t::SwsContext_t_sp_type m_sws;
};

#endif
