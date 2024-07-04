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
public:
    explicit VideoDecoder(Cv_Any_Type &,PacketQueue &,AVCodecContext &);

};




#endif //PLAYER_VIDEODECODER_HPP
