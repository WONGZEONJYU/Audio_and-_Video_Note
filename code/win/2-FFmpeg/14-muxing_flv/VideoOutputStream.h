#ifndef INC_14_MUXING_FLV_VIDEOOUTPUTSTREAM_H
#define INC_14_MUXING_FLV_VIDEOOUTPUTSTREAM_H

#include "OutputStreamAbstract.h"
#include "SwsContext_t.h"
#include <memory>

class VideoOutputStream final : public OutputStreamAbstract {

    static inline constexpr auto STREAM_PIX_FMT{AV_PIX_FMT_YUV420P};
    static inline constexpr auto STREAM_FRAME_RATE{25};

    using VideoOutputStream_sp_type = std::shared_ptr<VideoOutputStream>;

    void fill_yuv_image(AVFrame &pict);
    AVFrame *alloc_picture();
    bool get_one_frame() noexcept(false);
    explicit VideoOutputStream(AVFormatContext &);
    bool construct() noexcept;
    void config_codec_params() override;
    bool open_video();
    bool sws_init() noexcept;

public:
    [[nodiscard]] bool write_frame() noexcept(false) override ;
    ~VideoOutputStream() override = default;
    static std::shared_ptr<OutputStreamAbstract> create(AVFormatContext &);

private:

    SwsContext_t::SwsContext_t_sp_type m_sws;
};

#endif
