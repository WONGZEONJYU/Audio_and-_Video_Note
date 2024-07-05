//
// Created by Administrator on 2024/7/4.
//

#ifndef PLAYER_VIDEODECODER_HPP
#define PLAYER_VIDEODECODER_HPP

#include "DecoderAbstract.hpp"

class VideoDecoder:public DecoderAbstract{

    void av_decoder_thread(void *) override;
    int get_video_frame(AVFrame *);
    static int queue_picture(FrameQueue *,
                      AVFrame*,
                      const double &,
                      const double &,
                      const int64_t &,
                      const int &);
    explicit VideoDecoder(Cv_Any_Type &,PacketQueue &,AVCodecContext &);
    friend class std::shared_ptr<VideoDecoder> new_VideoDecoder(std::condition_variable_any &,
            PacketQueue &,
            AVCodecContext &) noexcept(false);
};

using VideoDecoder_sptr = std::shared_ptr<VideoDecoder>;
VideoDecoder_sptr new_VideoDecoder(std::condition_variable_any &,PacketQueue &,AVCodecContext &) noexcept(false);

#endif
