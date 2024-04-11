#include "Muxing_FLV.h"

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/timestamp.h>
}

#include <iostream>
#include <utility>
#include "AVHelper.h"

/* Prepare a 16 bit dummy audio frame of 'frame_size' samples and
 * 'nb_channels' channels. */

Muxing_FLV::Muxing_FLV(std::string filename):m_filename(std::move(filename)){}

bool Muxing_FLV::construct() noexcept {

   auto ret{avformat_alloc_output_context2(&m_fmt_ctx, nullptr, nullptr, m_filename.c_str())};

    if (ret < 0) {
        std::cerr << "avformat_alloc_output_context2 failed code : " << ret << "\t" << AVHelper::av_get_err(ret) << "\n";
        std::cerr << "Could not deduce output format from file extension: using flv.\n";
        ret = avformat_alloc_output_context2(&m_fmt_ctx, nullptr, "flv", m_filename.c_str());

        if (ret < 0){
            std::cerr << "avformat_alloc_output_context2 failed : " << AVHelper::av_get_err(ret) << "\n";
            return false;
        }
    }

    auto output_fmt{const_cast<AVOutputFormat*>(m_fmt_ctx->oformat)};
    output_fmt->audio_codec = static_cast<AVCodecID>(AV_CODEC_ID_AAC);
    output_fmt->video_codec = static_cast<AVCodecID>(AV_CODEC_ID_H264);


    return true;
}

Muxing_FLV::Muxing_FLV_sp_type Muxing_FLV::create(const std::string & filename) noexcept(false) {

    try {
        Muxing_FLV_sp_type obj(new Muxing_FLV(filename));

        if (!obj->construct()){
            obj.reset();
            throw std::runtime_error("Muxing_FLV construct failed\n");
        }
        return obj;
    } catch (std::bad_alloc &e) {
        std::cerr << e.what() << "\n";
        throw std::runtime_error("alloc Muxing_FLV mem failed\n");
    }
}

void Muxing_FLV::destory()
{
    if (m_fmt_ctx){
        if (!(m_fmt_ctx->flags & AVFMT_NOFILE)) {
            avio_close(m_fmt_ctx->pb);
        }

        avformat_free_context(m_fmt_ctx);
        m_fmt_ctx = nullptr;
    }
}

Muxing_FLV::~Muxing_FLV() {
    destory();
}

bool Muxing_FLV::add_stream() {


    return false;
}

void Muxing_FLV::init_codec_parms() {

}


