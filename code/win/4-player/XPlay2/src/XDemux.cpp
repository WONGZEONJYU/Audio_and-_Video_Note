//
// Created by wong on 2024/7/27.
//

extern "C"{
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavutil/channel_layout.h>
}

using namespace std;

#include "XDemux.hpp"
#include "AVHelper.h"

atomic_uint64_t XDemux:: sm_init_times{};
mutex XDemux::sm_mux;

XDemux::XDemux() {

    if (!sm_init_times){
        cerr << __func__  << "\n";
        unique_lock lock(sm_mux);
        avformat_network_init();
    }
    ++sm_init_times;
}

XDemux::~XDemux() {
    avformat_close_input(&m_av_fmt_ctx);
    if (!(--sm_init_times)){
        cerr << __func__  << "\n";
        unique_lock lock(sm_mux);
        avformat_network_deinit();
    }
}

void XDemux::Open(const string &url) noexcept(false){

    if (is_init){
        cerr << url << " reopen!\n";
        return;
    }

    AVDictionary *opts{};
    unique_lock lock(m_mux);
    Destroyer d([&opts]{
        cerr << "av_dict_free\n";
        av_dict_free(&opts);
    });

    try {
        FF_CHECK_ERR(av_dict_set(&opts, "rtsp_transport", "tcp", 0));
        FF_CHECK_ERR(av_dict_set(&opts, "max_delay", "500", 0));

        FF_CHECK_ERR(avformat_open_input(&m_av_fmt_ctx,url.c_str(), nullptr, &opts));
        FF_CHECK_ERR(avformat_find_stream_info(m_av_fmt_ctx, nullptr));
        m_audio_stream_index = av_find_best_stream(m_av_fmt_ctx,AVMEDIA_TYPE_AUDIO,-1,-1, nullptr,0);
        m_video_stream_index = av_find_best_stream(m_av_fmt_ctx,AVMEDIA_TYPE_VIDEO,-1,-1, nullptr,0);

        if (-1 == m_audio_stream_index && -1 == m_video_stream_index){
            throw runtime_error("no audio_stream and no video_stream!\n");
        }

        m_audio_stream = m_av_fmt_ctx->streams[m_audio_stream_index];
        m_video_stream = m_av_fmt_ctx->streams[m_video_stream_index];
        m_totalMS = m_av_fmt_ctx->duration / AV_TIME_BASE / 1000;

        av_dump_format(m_av_fmt_ctx,0,url.c_str(),0);
        cerr << "\n\n";

        show_audio_info();
        show_video_info();
        is_init = true;
    } catch (...) {
        avformat_close_input(&m_av_fmt_ctx);;
        d.destroy();
        lock.unlock();
        rethrow_exception(current_exception());
    }
}

void XDemux::show_audio_info() const noexcept(true) {

    if (!m_audio_stream){
        cerr << "no audio\n";
        return;
    }

    cerr << "=======================audio_info========================\n";
    const auto codec{m_audio_stream->codecpar};

    const double duration{ AV_NOPTS_VALUE !=  m_audio_stream->duration ? static_cast<double >(m_audio_stream->duration) *
                                    av_q2d(m_audio_stream->time_base) : NAN};

    const auto hour{isnan(duration) ? NAN : duration / 3600.0};
    const auto minute{isnan(duration) ? NAN : static_cast<double >(static_cast<uint64_t>(duration) % 3600) / 60.0 };
    const auto Second{isnan(duration) ? NAN : static_cast<double >(static_cast<uint64_t>(duration) % 60) };


    cerr << "audio_stream_index: " << m_audio_stream_index << "\n" <<
        "codec_id: " << codec->codec_id << "\n" <<
        "codec_name: " << avcodec_get_name(codec->codec_id) << "\n" <<
        "audio_format: " << av_get_sample_fmt_name(static_cast<AVSampleFormat>(codec->format)) << "\n" <<
        "sample_rate: " << codec->sample_rate << "\n" <<
        "channel_layout: " << AVHelper::channel_layout_describe(codec->ch_layout) << "\n" <<
        "frame_size: " << codec->frame_size << "\n" <<
        "duration: " << (isnan(duration) ? "unknown" :
                        to_string(hour) + ":" + to_string(minute) + ":" + to_string(Second)) << "\n" <<
        "bit_rate: " << codec->bit_rate << "\n";
    cerr << "=======================audio_info========================\n\n";
}

void XDemux::show_video_info() const noexcept(true) {

    if (!m_video_stream){
        cerr << "no video\n";
        return;
    }

    cerr << "=======================video_info========================\n";
    const auto codec{m_video_stream->codecpar};

    const double duration{ AV_NOPTS_VALUE !=  m_video_stream->duration ?
                            static_cast<double >(m_video_stream->duration) * av_q2d(m_video_stream->time_base) : NAN};
    const auto hour {isnan(duration) ? NAN : duration / 3600.0};
    const auto minute { isnan(duration) ? NAN : static_cast<double>(static_cast<uint64_t>(duration) % 3600) / 60.0 };
    const auto Second {isnan(duration) ? NAN : static_cast<double>(static_cast<uint64_t>(duration) % 60) };

    cerr << "video_stream_index: " << m_video_stream_index << "\n" <<
         "codec_id: " << codec->codec_id << "\n" <<
         "codec_name: " << avcodec_get_name(codec->codec_id) << "\n" <<
         "video_format: " << av_get_pix_fmt_name(static_cast<AVPixelFormat>(codec->format)) << "\n" <<
         "frame_rate: " << codec->framerate.num << "\n" <<
         "duration: " << (isnan(duration) ? "unknown" :
                          to_string(hour) + ":" + to_string(minute) + ":" + to_string(Second)) << "\n" <<
         "width: " << codec->width << "\n" <<
         "height: " << codec->height << "\n" <<
         "bit_rate: " << codec->bit_rate << "\n";

    cerr << "=======================video_info========================\n\n";
}
