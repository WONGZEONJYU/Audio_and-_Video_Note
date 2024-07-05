//
// Created by Administrator on 2024/7/4.
//

#include "AudioDecoder.hpp"
#include "FFPlay.hpp"
#include "ShareAVFrame.hpp"
#include "AVHelper.h"

AudioDecoder::AudioDecoder(DecoderAbstract::Cv_Any_Type &cv,
                           PacketQueue &q,
                           AVCodecContext &av_codec_ctx) :
DecoderAbstract(cv,q,av_codec_ctx) {

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
        throw std::runtime_error("new AudioDecoder failed");
    }
}

void AudioDecoder::av_decoder_thread(void *o) {

    std::cerr << __FUNCTION__ << "begin\n";

    auto obj{static_cast<FFPlay*>(o)};
    ShareAVFrame_sp_type frame;

    try {
        frame = new_ShareAVFrame();
        int ret{};
        Frame *af{};
        do {
            auto got_frame {decode_frame(*frame)};
            if (got_frame < 0){
                throw std::runtime_error(std::string(__FUNCTION__ ) + " " + std::to_string(__LINE__) + " " + AVHelper::av_get_err(got_frame) + "\n");
            }

            if (!(af = frame_queue_peek_writable(obj->f_audio_frame_q()))){
                throw std::runtime_error(std::string(__FUNCTION__ ) + " " + std::to_string(__LINE__) + " " + AVHelper::av_get_err(got_frame) + "\n");
            }

            const AVRational tb {1,(*frame)->sample_rate};
            const auto pts {(*frame)->pts};
            af->pts = (AV_NOPTS_VALUE == pts) ? NAN : static_cast<double>(pts) * av_q2d(tb);
            af->duration = av_q2d({(*frame)->nb_samples, (*frame)->sample_rate});

            av_frame_move_ref(af->frame,*frame);
            frame_queue_push(obj->f_audio_frame_q());

        } while (ret >= 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF);

    } catch (const std::exception &e) {
        frame.reset();
        std::cerr << e.what() << "\n";
    }

    std::cerr << __FUNCTION__ << "end\n";
}

