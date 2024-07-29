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
#include "XAVPacket.hpp"

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
    Deconstruct();
    if (!(--sm_init_times)){
        cerr << "sm_init_times = " << sm_init_times << "\n";
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
    Destroyer d([&opts]{
        av_dict_free(&opts);
    });

    unique_lock lock(m_mux);

    try {
        FF_CHECK_ERR(av_dict_set(&opts, "rtsp_transport", "tcp", 0));
        FF_CHECK_ERR(av_dict_set(&opts, "max_delay", "500", 0));

        FF_CHECK_ERR(avformat_open_input(&m_av_fmt_ctx,url.c_str(), nullptr, &opts));
        FF_CHECK_ERR(avformat_find_stream_info(m_av_fmt_ctx, nullptr));

        if (!m_av_fmt_ctx->nb_streams){
            throw runtime_error("no audio_stream and no video_stream!\n");
        }

        m_nb_streams = m_av_fmt_ctx->nb_streams;
        m_streams = m_av_fmt_ctx->streams;

        CHECK_EXC(m_stream_indices = new uint32_t[m_av_fmt_ctx->nb_streams]{});

        for (uint32_t i {}; i < m_nb_streams; ++i) {
            m_stream_indices[i] = i;
        }

        m_totalMS = m_av_fmt_ctx->duration / AV_TIME_BASE / 1000;

        av_dump_format(m_av_fmt_ctx,0,url.c_str(),0);
        cerr << "\n\n";

        show_audio_info();
        show_video_info();
        is_init = true; //同一个类不允许重复打开同一个文件
    } catch (...) {
        Deconstruct();
        d.destroy();
        lock.unlock();
        rethrow_exception(current_exception());
    }
}

void XDemux::show_audio_info() const noexcept(true) {

    cerr << "=======================audio_info========================\n";
    bool b{};
    for (uint32_t i {}; i < m_nb_streams; ++i) {

        const auto codec{m_streams[i]->codecpar};

        if (AVMEDIA_TYPE_AUDIO == codec->codec_type){

            const double duration{ AV_NOPTS_VALUE !=  m_streams[i]->duration ? static_cast<double >(m_streams[i]->duration) *
                                                                                 av_q2d(m_streams[i]->time_base) : NAN};
            const auto hour{isnan(duration) ? NAN : duration / 3600.0};
            const auto minute{isnan(duration) ? NAN : static_cast<double >(static_cast<uint64_t>(duration) % 3600) / 60.0 };
            const auto Second{isnan(duration) ? NAN : static_cast<double >(static_cast<uint64_t>(duration) % 60) };
            const auto tag{av_dict_get(m_streams[i]->metadata, "handler_name", nullptr, 0)};

            cerr << "audio_stream_index: " << i << "\n" <<
                 "codec_id: " << codec->codec_id << "\n" <<
                 "codec_name: " << avcodec_get_name(codec->codec_id) << "\n" <<
                 "language: " << (tag ? tag->value : "unknown") << "\n" <<
                 "audio_format: " << av_get_sample_fmt_name(static_cast<AVSampleFormat>(codec->format)) << "\n" <<
                 "sample_rate: " << codec->sample_rate << "\n" <<
                 "channel_layout: " << AVHelper::channel_layout_describe(codec->ch_layout) << "\n" <<
                 "frame_size: " << codec->frame_size << "\n" <<
                 "duration: " << (isnan(duration) ? "unknown" :
                                  to_string(hour) + ":" + to_string(minute) + ":" + to_string(Second)) << "\n" <<
                 "bit_rate: " << codec->bit_rate << "\n\n";
            b = true;
        }
    }

    if (!b){
        cerr << "no audio\n";
    }

    cerr << "=======================audio_info========================\n\n";
}

void XDemux::show_video_info() const noexcept(true) {

    cerr << "=======================video_info========================\n";
    bool b{};
    for (uint32_t i {}; i < m_nb_streams; ++i) {

        const auto codec{m_streams[i]->codecpar};
        if (AVMEDIA_TYPE_VIDEO == codec->codec_type){

            const double duration{ AV_NOPTS_VALUE !=  m_streams[i]->duration ?
                                   static_cast<double >(m_streams[i]->duration) * av_q2d(m_streams[i]->time_base) : NAN};
            const auto hour {isnan(duration) ? NAN : duration / 3600.0};
            const auto minute { isnan(duration) ? NAN : static_cast<double>(static_cast<uint64_t>(duration) % 3600) / 60.0 };
            const auto Second {isnan(duration) ? NAN : static_cast<double>(static_cast<uint64_t>(duration) % 60) };

            cerr << "video_stream_index: " << i << "\n" <<
                 "codec_id: " << codec->codec_id << "\n" <<
                 "codec_name: " << avcodec_get_name(codec->codec_id) << "\n" <<
                 "video_format: " << av_get_pix_fmt_name(static_cast<AVPixelFormat>(codec->format)) << "\n" <<
                 "frame_rate: " << codec->framerate.num << "\n" <<
                 "duration: " << (isnan(duration) ? "unknown" :
                                  to_string(hour) + ":" + to_string(minute) + ":" + to_string(Second)) << "\n" <<
                 "width: " << codec->width << "\n" <<
                 "height: " << codec->height << "\n" <<
                 "bit_rate: " << codec->bit_rate << "\n";
            b = true;
        }
    }

    if (!b){
        cerr << "no video\n";
    }

    cerr << "=======================video_info========================\n\n";
}

XAVPacket_sptr XDemux::Read() noexcept(false) {

    XAVPacket_sptr packet;

    try {
        CHECK_NULLPTR(m_av_fmt_ctx);
        unique_lock lock(m_mux);
        packet = new_XAVPacket();
        const auto ret {av_read_frame(m_av_fmt_ctx,packet.get())};
        if (ret < 0){
            packet.reset();
        }else{
            const auto time_base {m_av_fmt_ctx->streams[packet->stream_index]->time_base};
            const auto pts{static_cast<double >(packet->pts)},
                    dst{static_cast<double >(packet->dts)};
            packet->pts = static_cast<int64_t >(pts * 1000.0 * av_q2d(time_base));
            packet->dts = static_cast<int64_t >(dst * 1000.0 * av_q2d(time_base));
            //cerr << "packet->stream_index: " << packet->stream_index << " pts: " << packet->pts << "\n";
        }
        return packet;
    } catch (...) {
        rethrow_exception(current_exception());
    }
}

void XDemux::Deconstruct() noexcept(true) {
    delete [] m_stream_indices;
    m_stream_indices = nullptr;
    avformat_close_input(&m_av_fmt_ctx);
}
