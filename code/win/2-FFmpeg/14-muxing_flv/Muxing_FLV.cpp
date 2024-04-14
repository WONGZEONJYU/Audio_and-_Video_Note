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

bool Muxing_FLV::construct() noexcept
{
    const auto ret {alloc_fmt_ctx()};

    if (!ret){
        return {};
    }

//    output_fmt.audio_codec = static_cast<AVCodecID>(AV_CODEC_ID_AAC);
//    output_fmt.video_codec = static_cast<AVCodecID>(AV_CODEC_ID_H264);

    try {
        video_output_stream = VideoOutputStream::create(*m_fmt_ctx);
        audio_output_stream = AudioOutputStream::create(*m_fmt_ctx);
    } catch (std::exception &e) {
        std::cerr << e.what() << "\n";
        return {};
    }

    av_dump_format(m_fmt_ctx, 0, m_filename.c_str(), 1);

    return open_media_file();
}

Muxing_FLV::Muxing_FLV_sp_type Muxing_FLV::create(const std::string &filename) noexcept(false)
{
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

Muxing_FLV::~Muxing_FLV() {
    std::cerr << __FUNCTION__ << "\n";
    if (m_fmt_ctx){
        if (!(m_fmt_ctx->flags & AVFMT_NOFILE)) {
            avio_closep(&m_fmt_ctx->pb);
        }

        avformat_free_context(m_fmt_ctx);
        m_fmt_ctx = nullptr;
    }
}

void Muxing_FLV::exec() noexcept{

    if (!write_header()){
        return;
    }

    auto video{true},audio{true};

    while (video || audio){

        try {
            // video_st.next_pts值 <= audio_st.next_pts时
            const auto min {av_compare_ts(video_output_stream->nex_pts(),video_output_stream->time_base(),
                                         audio_output_stream->nex_pts(),audio_output_stream->time_base()) <= 0};

            if (video && (!audio || min)){
                std::cerr << "write video\n";
                video = video_output_stream->write_frame();
            }else{
                std::cerr << "write audio\n";
                audio = audio_output_stream->write_frame();
            }

        }catch (const std::runtime_error &e){
            std::cerr << e.what() << "\n";
            return;
        }
    }

    write_trailer();
}

bool Muxing_FLV::alloc_fmt_ctx() {

    auto ret{avformat_alloc_output_context2(&m_fmt_ctx, nullptr, nullptr, m_filename.c_str())};

    if (ret < 0) {
        std::cerr << "avformat_alloc_output_context2 failed code : " << ret << "\t" << AVHelper::av_get_err(ret) << "\n";
        std::cerr << "Could not deduce output format from file extension: using flv.\n";
        ret = avformat_alloc_output_context2(&m_fmt_ctx, nullptr, "flv", m_filename.c_str());

        if (ret < 0){
            std::cerr << "avformat_alloc_output_context2 failed : " << AVHelper::av_get_err(ret) << "\n";
            return {};
        }
    }

    return true;
}

bool Muxing_FLV::open_media_file() {

    if (!(m_fmt_ctx->flags & AVFMT_NOFILE)){
        const auto ret{avio_open(&m_fmt_ctx->pb,m_filename.c_str(),AVIO_FLAG_WRITE)};
        if (ret < 0){
            std::cerr << "Could not open " << m_filename << " : " << AVHelper::av_get_err(ret) << "\n";
            return {};
        }
    }

    return true;
}

bool Muxing_FLV::write_header() {

    // audio AVstream->base_time = 1/44100, video AVstream->base_time = 1/25
    /* 写头部到底做了什么操作呢？ 对应steam的time_base被改写和封装格式有关系*/
    // base_time audio = 1/1000  , video = 1/1000
    const auto ret{avformat_write_header(m_fmt_ctx, nullptr)};
    if (ret < 0){
        std::cerr << "Error occurred when opening output file: " << AVHelper::av_get_err(ret) << "\n";
        return {};
    }

    return true;
}

bool Muxing_FLV::write_trailer() {

    const auto ret{av_write_trailer(m_fmt_ctx)};

    if(ret < 0){
        std::cerr << "av_write_trailer failed: " << AVHelper::av_get_err(ret) << "\n";
        return {};
    }

    return true;
}
