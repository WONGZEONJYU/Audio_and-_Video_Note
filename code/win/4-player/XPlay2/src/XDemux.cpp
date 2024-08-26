//
// Created by wong on 2024/7/27.
//

extern "C"{
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavutil/channel_layout.h>
}

#include "XDemux.hpp"
#include "XAVPacket.hpp"
#include "XAVCodecParameters.hpp"
#include <iostream>
#include <string>

using namespace std;

atomic_uint64_t XDemux:: sm_init_times{};
mutex XDemux::sm_mux;

int XDemux::io_callback(void *args) {
    auto this_{static_cast<XDemux*>(args)};
    cerr << this_ << "\n";
    return this_->m_is_exit;
}

XDemux::XDemux() {

    if (!sm_init_times){
        unique_lock lock(sm_mux);
        avformat_network_init();
    }
    ++sm_init_times;
}

XDemux::~XDemux() {
    DeConstruct();
    if (!(--sm_init_times)){
        unique_lock lock(sm_mux);
        avformat_network_deinit();
    }
}

void XDemux::Find_Media(const int &media_type,
                        std::atomic_int &index,
                        AVStream *&st) noexcept(true) {

    for(const auto &item:m_stream_indices) {
        const auto type {m_streams[item]->codecpar->codec_type};
        if (static_cast<AVMediaType>(media_type) == type) {
            index = item;
            st = m_streams[item];
            break;
        }
    }
}

void XDemux::Open(const string &url) noexcept(false){
    Close();
    AVDictionary *opts{};
    Destroyer d([&opts]{
        av_dict_free(&opts);
    });

    try {
        FF_CHECK_ERR(av_dict_set(&opts, GET_STR(rtsp_transport), GET_STR(tcp), 0));
        FF_CHECK_ERR(av_dict_set(&opts, GET_STR(max_delay), GET_STR(500), 0));
    } catch (...) {
        d.destroy();
        throw;
    }

    unique_lock lock(m_mux);
    try {
        FF_CHECK_ERR(avformat_open_input(&m_av_fmt_ctx,url.c_str(), nullptr, &opts));
        FF_CHECK_ERR(avformat_find_stream_info(m_av_fmt_ctx, nullptr));
        m_av_fmt_ctx->interrupt_callback.callback = io_callback;
        m_av_fmt_ctx->interrupt_callback.opaque = this;
        m_nb_streams = m_av_fmt_ctx->nb_streams;
        m_streams = m_av_fmt_ctx->streams;
        if (!m_nb_streams || !m_streams) {
            throw runtime_error(GET_STR(no audio_stream and no video_stream!\n));
        }

        m_stream_indices.resize(m_nb_streams,-1);

        for (int i {};auto &item:m_stream_indices) {
            item = i;
            ++i;
        }

#if 0
        m_Present_Video_index = av_find_best_stream(m_av_fmt_ctx,AVMEDIA_TYPE_VIDEO,-1,-1,nullptr,0);
        m_Present_Video_st = m_Present_Video_index >= 0 ? m_streams[m_Present_Video_index] : nullptr;

        m_Present_Audio_index = av_find_best_stream(m_av_fmt_ctx,AVMEDIA_TYPE_AUDIO,-1,-1,nullptr,0);
        m_Present_Audio_st = m_Present_Audio_index >= 0 ? m_streams[m_Present_Audio_index] : nullptr;
#else
        Find_Media(AVMEDIA_TYPE_AUDIO,m_Present_Audio_index,m_Present_Audio_st);
        Find_Media(AVMEDIA_TYPE_VIDEO,m_Present_Video_index,m_Present_Video_st);
#endif
        m_totalMS = m_av_fmt_ctx->duration / (AV_TIME_BASE / 1000);

        av_dump_format(m_av_fmt_ctx,0,url.c_str(),0);
        cerr << "\n\n";
        show_audio_info();
        show_video_info();

        cerr << GET_STR(Present_Video_index: ) <<
                (m_Present_Video_index >= 0 ? to_string(m_Present_Video_index) : GET_STR(does not exist)) << "\n" <<
                 GET_STR(Present_Audio_index: ) <<
                 (m_Present_Audio_index >= 0 ? to_string(m_Present_Audio_index) : GET_STR(does not exist)) << "\n\n";

        //cerr << "m_av_fmt_ctx->iformat->name : " << m_av_fmt_ctx->iformat->name << "\n";

    } catch (...) {
        DeConstruct();
        lock.unlock();
        d.destroy();
        throw;
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

            const auto tag{av_dict_get(m_streams[i]->metadata, GET_STR(handler_name), nullptr, 0)};
            const auto handler_name {tag ? tag->value : nullptr};
            const auto language {handler_name ? handler_name : GET_STR(unknown)};

            cerr << "audio_stream_index: " << i << "\n" <<
                 "codec_id: " << codec->codec_id << "\n" <<
                 "codec_name: " << avcodec_get_name(codec->codec_id) << "\n" <<
                 "language: " << language << "\n" <<
                 "audio_format: " << av_get_sample_fmt_name(static_cast<AVSampleFormat>(codec->format)) << "\n" <<
                 "sample_rate: " << codec->sample_rate << "\n" <<
                 "channel_layout: " << XHelper::channel_layout_describe(codec->ch_layout) << "\n" <<
                 "frame_size: " << codec->frame_size << "\n" <<
                 "duration: " << (isnan(duration) ? GET_STR(unknown) :
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

void XDemux::show_video_info()  noexcept(true) {

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

    unique_lock lock(m_mux);
    if (!m_av_fmt_ctx){
        PRINT_ERR_TIPS(GET_STR(Please initialize first));
        return {};
    }

    int ret;
    FF_ERR_OUT(ret = avio_feof(m_av_fmt_ctx->pb));
    if (ret < 0){
        return {};
    }

    XAVPacket_sptr packet;
    CHECK_EXC(packet = new_XAVPacket(),lock.unlock());
    //ret = av_read_frame(m_av_fmt_ctx,packet.get());
    FF_ERR_OUT(ret = av_read_frame(m_av_fmt_ctx,packet.get()));
    if (ret < 0){
        packet.reset();
    }else{
        const auto time_base {m_streams[packet->stream_index]->time_base};
        lock.unlock();

        if (AV_NOPTS_VALUE == packet->pts){
//            m_last_pts += 40;
//            packet->pts = m_last_pts;
        }else{
            const auto pts{static_cast<double>(packet->pts)},
                    dst{static_cast<double>(packet->dts)};
            packet->pts = static_cast<decltype(packet->pts)>(pts * 1000.0 * av_q2d(time_base));
            packet->dts = static_cast<decltype(packet->dts)>(dst * 1000.0 * av_q2d(time_base));
        }
    }
    return packet;
}

XAVPacket_sptr XDemux::ReadVideo() noexcept(false) {

    XAVPacket_sptr packet;
    for (uint32_t i {}; i < 20; ++i) {
        packet = Read();
        if (!packet) {break;}
        if (m_Present_Video_index == packet->stream_index) {
            break;
        }
    }
    return packet;
}

void XDemux::DeConstruct() noexcept(true) {
    m_stream_indices.clear();
    avformat_close_input(&m_av_fmt_ctx);
    m_streams = nullptr;
    m_Present_Video_st = m_Present_Audio_st = nullptr;
    m_last_pts = m_totalMS = m_nb_streams = 0;
    m_Present_Video_index = m_Present_Audio_index = -1;
}

XAVCodecParameters_sptr XDemux::Copy_Present_VideoCodecParam() noexcept(false) {

    unique_lock lock(m_mux);
    if (!m_av_fmt_ctx){
        PRINT_ERR_TIPS(GET_STR(Please initialize first));
        return {};
    }

    if (!m_Present_Video_st){
        PRINT_ERR_TIPS(GET_STR(no video));
        return {};
    }

    XAVCodecParameters_sptr p;
    CHECK_EXC(p = new_XAVCodecParameters(m_Present_Video_st->codecpar),lock.unlock());
    return p;
}

XAVCodecParameters_sptr XDemux::Copy_Present_AudioCodecParam() noexcept(false) {

    unique_lock lock(m_mux);
    if (!m_av_fmt_ctx){
        PRINT_ERR_TIPS(GET_STR(Please initialize first));
        return {};
    }

    if (!m_Present_Audio_st){
        PRINT_ERR_TIPS(GET_STR(no audio));
        return {};
    }

    XAVCodecParameters_sptr p;
    CHECK_EXC(p = new_XAVCodecParameters(m_Present_Audio_st->codecpar),lock.unlock());
    return p;
}

bool XDemux::is_Audio(const XAVPacket_sptr &pkt) noexcept(true){

    if (!pkt){
        PRINT_ERR_TIPS(GET_STR(XAVPacket_sptr is empty));
        return {};
    }

    unique_lock lock(m_mux);
    return m_Present_Video_index != pkt->stream_index;
}

bool XDemux::Seek(const double &pos) noexcept(true) {
    unique_lock lock(m_mux);
    if (!m_av_fmt_ctx){
        PRINT_ERR_TIPS(GET_STR(Please initialize first));
        return {};
    }

    if (!m_Present_Video_st || m_Present_Video_index < 0) {
        PRINT_ERR_TIPS(GET_STR(no video));
        return {};
    }

    const auto SeekPos{AV_NOPTS_VALUE == m_Present_Video_st->duration ?
                    static_cast<int64_t>(static_cast<double>(m_av_fmt_ctx->duration) * pos):
                    static_cast<int64_t>(static_cast<double >(m_Present_Video_st->duration) * pos)};

    FF_ERR_OUT(avformat_flush(m_av_fmt_ctx));
    int ret;
//    FF_ERR_OUT(ret = av_seek_frame(m_av_fmt_ctx,m_Present_Video_index,
//                                SeekPos,AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME));

    FF_ERR_OUT(ret = avformat_seek_file(m_av_fmt_ctx, m_Present_Video_index, INT64_MIN, SeekPos, INT64_MAX, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME));
    return ret >= 0;
}

void XDemux::Clear() noexcept(true) {
    unique_lock lock(m_mux);
    if (!m_av_fmt_ctx){
        PRINT_ERR_TIPS(GET_STR(Please initialize first));
        return;
    }
    FF_ERR_OUT(avformat_flush(m_av_fmt_ctx));
}

void XDemux::Close() noexcept(true) {
    unique_lock lock(m_mux);
    DeConstruct();
}
