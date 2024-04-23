//
// Created by Administrator on 2024/4/18.
//

#ifndef INC_15_MP4_MUXER_VIDEOENCODER_H
#define INC_15_MP4_MUXER_VIDEOENCODER_H

#include "EncoderAbstract.h"

struct Video_Encoder_params{

    explicit Video_Encoder_params(const int &width,
                         const int &height,
                         const int &gop_size = 25,
                         const int &max_b_frames = 0,
                         const AVPixelFormat &pix_fmt = AV_PIX_FMT_YUV420P,
                         const AVRational &time_base = {1,25},
                         const int64_t &bit_rate = 500*1024,
                         const AVCodecID &CodecID = AV_CODEC_ID_H264,
                         const int &flags = AV_CODEC_FLAG_GLOBAL_HEADER) noexcept(true):
            m_width(width),m_height(height),m_gop_size(gop_size),max_b_frames(max_b_frames),m_flags(flags),
            m_pix_fmt{pix_fmt}, m_time_base(time_base),m_bit_rate(bit_rate),m_Codec_ID(CodecID){}

   int m_width{},m_height{},m_gop_size{},max_b_frames{},m_flags{};
   AVPixelFormat m_pix_fmt{};
   AVRational m_time_base{};
   int64_t m_bit_rate{};
   AVCodecID m_Codec_ID{};
};

class VideoEncoder final : public EncoderAbstract {

    void Construct(const Video_Encoder_params &) noexcept(false);
    explicit VideoEncoder() = default;
    void init_codec(const Video_Encoder_params &) noexcept(false);
public:
    using VideoEncoder_sp_type = std::shared_ptr<VideoEncoder>;
    static VideoEncoder_sp_type create(const Video_Encoder_params &) noexcept(false);
};

using VideoEncoder_sp_type = typename VideoEncoder::VideoEncoder_sp_type;

VideoEncoder_sp_type new_VideoEncoder(const Video_Encoder_params &) noexcept(false);

#endif
