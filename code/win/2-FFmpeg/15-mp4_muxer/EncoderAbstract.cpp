//
// Created by Administrator on 2024/4/18.
//

#include "EncoderAbstract.h"
#include "AVHelper.h"

void EncoderAbstract::encode(const std::string &name,
                             const ShareAVFrame_sp_type &frame,
                             const int &stream_index,
                             const long long int &pts,
                             const AVRational &time_base,
                             vector_type &packets) const noexcept(false)
{
    AVFrame *in_frame{};
    if (frame){
        frame->m_frame->pts = av_rescale_q(pts,time_base,m_codec_ctx->time_base);
        in_frame = frame->m_frame;
    }

    try {
        AVHelper::encode(name, m_codec_ctx, in_frame,[&](auto &pkt) {
            pkt->m_packet->stream_index = stream_index;
            packets.push_back(std::move(pkt));
        });

    } catch (const std::runtime_error &e) {
        throw std::runtime_error(e.what());
    }
}

EncoderAbstract::~EncoderAbstract() {
    std::cerr << __FUNCTION__ << "\t" << m_codec_ctx->codec->name << "\n";
    avcodec_free_context(&m_codec_ctx);
}

void EncoderAbstract::parameters_from_context(AVCodecParameters *par) noexcept(false){

    const auto ret {avcodec_parameters_from_context(par,m_codec_ctx)};
    if (ret < 0){
        throw std::runtime_error("avcodec_parameters_from_context failed: " + AVHelper::av_get_err(ret) + "\n");
    }
}
