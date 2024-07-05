//
// Created by Administrator on 2024/7/4.
//

#ifndef PLAYER_AUDIODECODER_HPP
#define PLAYER_AUDIODECODER_HPP

#include "DecoderAbstract.hpp"

class AudioDecoder : public DecoderAbstract {

    explicit AudioDecoder(Cv_Any_Type &,PacketQueue &,AVCodecContext &);
    void av_decoder_thread(void *) override;
    friend class std::shared_ptr<AudioDecoder> new_AudioDecoder(std::condition_variable_any &
            ,PacketQueue &,
            AVCodecContext &) noexcept(false);
};

using AudioDecoder_sptr = std::shared_ptr<AudioDecoder>;
AudioDecoder_sptr new_AudioDecoder(std::condition_variable_any &,
                                   PacketQueue &,
                                   AVCodecContext &) noexcept(false);
#endif
