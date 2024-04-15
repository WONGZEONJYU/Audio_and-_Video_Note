
#include "OutputStreamAbstract.h"

#include "AVHelper.h"

void OutputStreamAbstract::write_media_file(AVPacket &pkt) noexcept(false)
{
    /* rescale output packet timestamp values from codec to stream timebase */
    // 将packet的timestamp由codec to stream timebase pts_before = -1024

    av_packet_rescale_ts(&pkt, m_codec_ctx->time_base, m_stream->time_base);

    pkt.stream_index = m_stream->index;
    // pts_before * 1/44100 = pts_after *1/1000
    // pts_after = pts_before * 1/44100 * 1000 = -1024 * 1/44100 * 1000 = -23
    /* Write the compressed frame to the media file. */

    AVHelper::log_packet(m_fmt_ctx,pkt);
    const auto ret {av_interleaved_write_frame(&m_fmt_ctx, &pkt)};
    if (ret < 0){
        throw std::runtime_error("av_interleaved_write_frame failed: " + std::to_string(ret) +
                                "\t" + AVHelper::av_get_err(ret) + "\n");
    }
}

void OutputStreamAbstract::add_stream(const int& codec_id) noexcept(false)
{
    /* 查找编码器 */
    m_codec = avcodec_find_encoder(static_cast<AVCodecID>(codec_id));
    if (!m_codec){
        throw std::runtime_error("Could not find encoder for " + std::string(avcodec_get_name(static_cast<AVCodecID>(codec_id))) + "\n");
    }

    m_stream = avformat_new_stream(&m_fmt_ctx, nullptr);
    if (!m_stream){
        throw std::runtime_error("Could not allocate stream\n");
    }

    m_stream->id = static_cast<int>(m_fmt_ctx.nb_streams - 1);

    m_codec_ctx = avcodec_alloc_context3(m_codec);
    if (!m_codec_ctx){
        throw std::runtime_error("Could not alloc an encoding context\n");
    }

    config_codec_params();
}

OutputStreamAbstract::OutputStreamAbstract(AVFormatContext &oc) noexcept :m_fmt_ctx(oc) {}

OutputStreamAbstract::~OutputStreamAbstract() {
    std::cerr << __FUNCTION__ << "\n";
    avcodec_free_context(&m_codec_ctx);
    av_frame_free(&m_frame);
    av_frame_free(&m_tmp_frame);
}

AVRational OutputStreamAbstract::time_base()  const noexcept(true){
    return m_codec_ctx->time_base;
}
