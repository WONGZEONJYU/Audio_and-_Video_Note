//
// Created by Administrator on 2024/4/18.
//

extern "C"{
#include <libavcodec/avcodec.h>
}

#include "EncoderAbstract.h"
#include "AVHelper.h"

void EncoderAbstract::encode(const std::string &name, const ShareAVFrame_sp_type &frame, const int &stream_index, const long long int &pts,
                             const AVRational &time_base,vector_type &packets) const noexcept(false) {

    auto pkt{ShareAVPacket::create()};

    if (frame){
        frame->m_frame->pts = av_rescale_q(pts,time_base,m_codec_ctx->time_base);
    }

    AVHelper::encode(name,m_codec_ctx,frame->m_frame,pkt->m_packet,[&]{
        pkt->m_packet->stream_index = stream_index;
        packets.push_back(std::move(pkt));
    });
}

EncoderAbstract::~EncoderAbstract() {
    std::cerr << __FUNCTION__ << "\n";
    avcodec_free_context(&m_codec_ctx);
}

void EncoderAbstract::parameters_from_context(AVCodecParameters *par) noexcept(false){

    const auto ret {avcodec_parameters_from_context(par,m_codec_ctx)};
    if (ret < 0){
        throw std::runtime_error("avcodec_parameters_from_context failed: " + AVHelper::av_get_err(ret) + "\n");
    }
}
