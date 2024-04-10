#include "Muxing_FLV.h"

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/timestamp.h>
}

#include <iostream>
#include <utility>
#include "AVHelper.h"

Muxing_FLV::OutputStream::~OutputStream(){
    avcodec_free_context(&m_codecContext);
    av_frame_free(&m_frame);
    av_frame_free(&m_tmp_frame);
}

void Muxing_FLV::fill_yuv_image(AVFrame &pict, const int &frame_index,
                           const int &width, const int &height)
{
    /* Y */
    for (int y {}; y < height; y++){
        for (int x {}; x < width; x++){
            pict.data[0][y * pict.linesize[0] + x] = x + y + frame_index * 3;
        }
    }

    /* Cb and Cr */
    for (int y {}; y < height / 2; y++)
    {
        for (int x {}; x < width / 2; x++) {
            pict.data[1][y * pict.linesize[1] + x] = 128 + y + frame_index * 2;
            pict.data[2][y * pict.linesize[2] + x] = 64 + x + frame_index * 5;
        }
    }
}

/* Prepare a 16 bit dummy audio frame of 'frame_size' samples and
 * 'nb_channels' channels. */
AVFrame *Muxing_FLV::get_audio_frame(Muxing_FLV::OutputStream &ost)
{
    auto frame{ost.m_tmp_frame};

    auto q {reinterpret_cast<int16_t*>(frame->data[0])};

    /* check if we want to generate more frames */
    // 44100 * {1, 44100} = 1 --> 44100*5 * {1, 44100} = 5
    // 5 *{1,1} = 5
    if (av_compare_ts(ost.m_next_pts, ost.m_codecContext->time_base,
                      STREAM_DURATION, { 1, 1 }) >= 0) {
        return nullptr;
    }

    for (int j {}; j < frame->nb_samples; j++) {

        const auto v {static_cast<int16_t>(sin(ost.m_t) * 10000)};

        for (int i {}; i < ost.m_codecContext->ch_layout.nb_channels; i++){
            *q++ = v;
        }

        ost.m_t     += ost.m_tincr;
        ost.m_tincr += ost.m_tincr2;
    }

    frame->pts = ost.m_next_pts; // 使用samples作为计数 设置pts 0, nb_samples(1024) 2048
    ost.m_next_pts += frame->nb_samples;    // 音频PTS使用采样点个数nb_samples叠加
    return frame;
}

Muxing_FLV::Muxing_FLV(std::string filename):
m_filename(std::move(filename)){}

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


