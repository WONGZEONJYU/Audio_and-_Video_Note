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

using namespace std;

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

    FF_CHECK_ERR(av_dict_set(&opts, "rtsp_transport", "tcp", 0));
    FF_CHECK_ERR(av_dict_set(&opts, "max_delay", "500", 0));
    unique_lock lock(m_mux);
    try {

        FF_CHECK_ERR(avformat_open_input(&m_av_fmt_ctx,url.c_str(), nullptr, &opts));
        FF_CHECK_ERR(avformat_find_stream_info(m_av_fmt_ctx, nullptr));

        if (!m_av_fmt_ctx->nb_streams){
            throw runtime_error("no audio_stream and no video_stream!\n");
        }

        m_nb_streams = m_av_fmt_ctx->nb_streams;
        m_streams = m_av_fmt_ctx->streams;

        CHECK_EXC(m_stream_indices = new int[m_av_fmt_ctx->nb_streams]{});
        std::fill_n(m_stream_indices,m_av_fmt_ctx->nb_streams,-1);

        for (uint32_t i {}; i < m_nb_streams; ++i) {
            m_stream_indices[i] = static_cast<int>(i);
        }

        m_totalMS = m_av_fmt_ctx->duration / (AV_TIME_BASE / 1000);
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
                 "channel_layout: " << XHelper::channel_layout_describe(codec->ch_layout) << "\n" <<
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
    CHECK_NULLPTR(m_av_fmt_ctx);
    CHECK_EXC(packet = new_XAVPacket());
    unique_lock lock(m_mux);
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
}

void XDemux::Deconstruct() noexcept(true) {
    delete [] m_stream_indices;
    m_stream_indices = nullptr;
    avformat_close_input(&m_av_fmt_ctx);
    m_streams = nullptr;
    m_totalMS = 0;
    is_init = false;
    m_nb_streams = 0;
}

XAVCodecParameters_container_sprt XDemux::copy_ALLCodec_Parameters() noexcept(false) {

    unique_lock lock(m_mux);
    XAVCodecParameters_container_sprt c;
    try {
        CHECK_NULLPTR(m_av_fmt_ctx);
        CHECK_EXC(c = make_shared<XAVCodecParameters_sptr_container>());
        CHECK_EXC(c->resize(m_nb_streams));

        for (uint32_t i {}; auto &item: *c) {
            CHECK_EXC(item = new_XAVCodecParameters());
            CHECK_EXC(item->from_AVFormatContext(m_streams[i]->codecpar));
            ++i;
        }
        return c;
    } catch (...) {
        lock.unlock();
        c.reset();
        rethrow_exception(current_exception());
    }
}

bool XDemux::Seek(const double &pos) noexcept(true)
{
    unique_lock lock(m_mux);
    if (!m_av_fmt_ctx){
        cerr << __func__ << ": m_av_fmt_ctx is nullptr\n";
        return false;
    }

    auto video_stream_index{-1};
    for (int i {}; i < m_nb_streams; ++i) {
        if (AVMEDIA_TYPE_VIDEO == m_streams[i]->codecpar->codec_type){
            video_stream_index = i;
            break;
        }
    }

    if (video_stream_index < 0){
        cerr << __func__ << ": no video\n";
        return false;
    }

    int64_t SeekPos{};

    if (AV_NOPTS_VALUE == m_streams[video_stream_index]->duration){
        SeekPos = static_cast<int64_t>(static_cast<double>(m_av_fmt_ctx->duration) * pos);
    }else{
        SeekPos = static_cast<int64_t>(static_cast<double >(m_streams[video_stream_index]->duration) * pos);
    }

    avformat_flush(m_av_fmt_ctx);
    const auto re{av_seek_frame(m_av_fmt_ctx,video_stream_index,SeekPos,AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME)};
    if (re < 0){
        cerr << __func__ << ": " << XHelper::av_get_err(re);
        return false;
    }

    return true;
}

void XDemux::Clear() noexcept(true) {
    unique_lock lock(m_mux);
    if (!m_av_fmt_ctx){
        return;
    }
    avformat_flush(m_av_fmt_ctx);
}

void XDemux::Close() noexcept(true) {
    unique_lock lock(m_mux);
    if (!m_av_fmt_ctx){
        return;
    }
    Deconstruct();
}