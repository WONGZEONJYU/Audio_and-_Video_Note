//
// Created by Administrator on 2024/4/18.
//

#ifndef INC_15_MP4_MUXER_VIDEOENCODER_H
#define INC_15_MP4_MUXER_VIDEOENCODER_H

#include "EncoderAbstract.h"

struct AVCodecContext;

struct Encoder_params{


};

class VideoEncoder : public EncoderAbstract {

    void Construct() noexcept(false);
    void DeConstruct() noexcept(true);
    explicit VideoEncoder() = default;
    void init_codec() noexcept(false);
    void encode(AVFrame *frame,const int &stream_index,const long long &pts,
                const AVRational& time_base,std::vector<AVPacket*>& packets) const noexcept(false) override;

    [[nodiscard]] AVRational time_base() const noexcept(true) override;

public:
    using VideoEncoder_sp_type = std::shared_ptr<VideoEncoder>;
    static VideoEncoder_sp_type create() noexcept(false);
    ~VideoEncoder() override;

private:
    AVCodecContext *m_codec_ctx{};
};

#endif //INC_15_MP4_MUXER_VIDEOENCODER_H
