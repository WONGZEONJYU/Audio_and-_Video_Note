//
// Created by Administrator on 2024/7/4.
//

#include "AudioDecoder.hpp"
#include "FFPlay.hpp"

AudioDecoder::AudioDecoder(DecoderAbstract::Cv_Any_Type &cv,
                           PacketQueue &q,
                           AVCodecContext &av_codec_ctx) :
DecoderAbstract(cv,q,av_codec_ctx){

}

AudioDecoder_sptr new_AudioDecoder(std::condition_variable_any &cv,
                                   PacketQueue &q,
                                   AVCodecContext &av_codec_ctx) noexcept(false)
{
    AudioDecoder_sptr obj;
    try {
        obj.reset(new AudioDecoder(cv,q,av_codec_ctx));
        return obj;
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw e;
    } catch (const std::bad_alloc &e) {
        obj.reset();
        throw e;
    }
}

void AudioDecoder::av_decoder_thread(void *o) {

    auto obj{static_cast<FFPlay*>(o)};

}

