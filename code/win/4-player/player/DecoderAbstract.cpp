//
// Created by Administrator on 2024/7/3.
//

#include "DecoderAbstract.hpp"

DecoderAbstract::DecoderAbstract(DecoderAbstract::Cv_Any_Type &cv,
                                 PacketQueue &q,
                                 AVCodecContext &av_codec_ctx):
m_cv{cv},m_queue{q},m_avcodec_ctx{&av_codec_ctx} {

}

void DecoderAbstract::Notify_All() noexcept(true) {
    m_cv.notify_all();
}

void DecoderAbstract::av_decoder_abort(FrameQueue &fq) noexcept(true){
    frame_queue_signal(&fq);
    packet_queue_abort(&m_queue);
    packet_queue_flush(&m_queue);
}

DecoderAbstract::~DecoderAbstract() {
    if (m_av_decode_thread.joinable()){
        m_av_decode_thread.join();
    }
    av_packet_unref(&m_pkt);
    avcodec_free_context(&m_avcodec_ctx);
}

void DecoderAbstract::av_decoder_start(void *args) noexcept(true){
    packet_queue_start(&m_queue);
    m_av_decode_thread = std::move(std::thread(&DecoderAbstract::av_decoder_thread,this,args));
}

int DecoderAbstract::decode_frame(AVFrame *frame) {

    auto ret {AVERROR(EAGAIN)};

    while (true){

        if (m_queue.abort_request){
            return -1;
        }

        do {
            switch (m_avcodec_ctx->codec_type) {

                case AVMEDIA_TYPE_AUDIO:
                    ret = avcodec_receive_frame(m_avcodec_ctx,frame);
                    if (ret >= 0){
                        const AVRational tb{1,frame->sample_rate};
                        if (AV_NOPTS_VALUE != frame->pts){
                            frame->pts = av_rescale_q(frame->pts,m_avcodec_ctx->time_base,tb); //转换成采样率
                        }
                    }
                    break;
                case AVMEDIA_TYPE_VIDEO:
                    ret = avcodec_receive_frame(m_avcodec_ctx,frame);
                    if (ret >= 0){
                        frame->pts = frame->best_effort_timestamp;
                    }
                    break;
                default:
                    break;
            }

            if (AVERROR_EOF == ret){
                avcodec_flush_buffers(m_avcodec_ctx);
                return 0;
            }

            if (ret >= 0){
                return 1;
            }

        } while (ret != AVERROR(EAGAIN));

        if (packet_queue_get(&m_queue,&m_pkt,1,&m_pkt_serial)< 0){ //阻塞读取
            return -1;
        }

        if (avcodec_send_packet(m_avcodec_ctx,&m_pkt) == AVERROR(EAGAIN)){

        } else{
            //av_packet_unref(&m_pkt);
        }
        av_packet_unref(&m_pkt);
    }
}
