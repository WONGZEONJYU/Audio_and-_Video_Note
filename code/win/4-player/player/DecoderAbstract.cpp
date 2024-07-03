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
