#ifndef INC_14_MUXING_FLV_VIDEOOUTPUTSTREAM_H
#define INC_14_MUXING_FLV_VIDEOOUTPUTSTREAM_H

#include "OutputStreamAbstract.h"
#include "SwsContext_t.h"
#include <memory>

struct AVFormatContext;
struct AVStream;
struct AVFrame;
struct AVCodec;
struct AVCodecContext;
enum AVPixelFormat;

class VideoOutputStream final : public OutputStreamAbstract {

    static inline constexpr auto STREAM_PIX_FMT{AV_PIX_FMT_YUV420P};
    static inline constexpr auto STREAM_FRAME_RATE{25};
    void fill_yuv_image(AVFrame &pict);
    AVFrame *alloc_picture();
    explicit VideoOutputStream(AVFormatContext &);
    bool construct();
    void init_codec_parms();
    bool add_stream();
    bool open();
    bool sws_init();

public:
    void write_frame() override;
    ~VideoOutputStream();
    static std::shared_ptr<OutputStreamAbstract> create(AVFormatContext &);

private:
    AVFormatContext &m_avFormatContext;
    const AVCodec * m_codec{};
    AVCodecContext *m_avCodecContext{};
    AVStream *m_stream{};
    AVFrame *m_frame{},*m_tmp_frame{};
    long long m_next_pts{};
    SwsContext_t::SwsContext_t_sp_type m_sws;
};

#endif