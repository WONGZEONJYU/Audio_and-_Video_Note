//
// Created by Administrator on 2024/6/27.
//

#ifndef PLAYER_FFPLAY_HPP
#define PLAYER_FFPLAY_HPP

#include <string>
#include <thread>
#include <atomic>
#include "MessageQueue.hpp"
#include "ff_ffplay_def.hpp"
#include "VideoDecoder.hpp"
#include "AudioDecoder.hpp"

class FFPlay final : protected MessageQueue {

    explicit FFPlay() = default;
    void construct() noexcept(false);
    void read_thread();
    void video_refresh_thread();
    void stream_open() noexcept(false);
    void stream_close() noexcept(true);
    void stream_component_open(const int&) noexcept(false);
    void stream_component_close(const int&);
    static int decode_interrupt_cb(void *);
    friend class std::shared_ptr<FFPlay> new_FFPlay() noexcept(false);

public:
    using MessageQueue::mq_start;
    using MessageQueue::mq_msg_put;
    using MessageQueue::mq_msg_get;
    ~FFPlay() override ;
    void prepare_async(const std::string &) noexcept(false);

    void f_start();
    void f_stop();
    [[nodiscard]] auto f_video_st() const{return m_video_st;}
    [[nodiscard]] auto f_audio_st() const{return m_audio_st;}
    [[nodiscard]] auto f_format_ctx() const {return m_ic;}

private:
    std::string m_url;
    std::thread m_read_th,m_video_refresh_th;
    std::condition_variable_any m_cv;
    std::atomic_bool m_abort_request{},m_eof{},m_muted{};

    int m_audio_stream {-1},m_video_stream{-1},
    m_av_sync_type{AV_SYNC_AUDIO_MASTER},
    m_startup_volume{100},
    m_audio_volume{};

    Clock m_audclk{};                   // 音频时钟
    Clock m_vidclk{};                   // 视频时钟
    Clock m_extclk{};                   // 外部时钟

    FrameQueue m_pictq{};               // 视频Frame队列
    //FrameQueue m_subpq;                 // 字幕Frame队列
    FrameQueue m_sampq{};               // 采样Frame队列


    PacketQueue m_videoq{};             // 视频队列
    //PacketQueue m_subtitleq{};          // 字幕packet队列
    PacketQueue m_audioq{};             // 音频packet队列

    AVStream *m_video_st{};             // 视频流
    //AVStream *m_subtitle_st{};          // 字幕流
    AVStream *m_audio_st{};             // 音频流

    AVFormatContext *m_ic{};

    AudioParams m_audio_src{},audio_filter_src{},audio_tgt{};

    VideoDecoder_sptr m_v_decoder;
    AudioDecoder_sptr m_a_decoder;

public:
    FFPlay(const FFPlay&) = delete;
    FFPlay(FFPlay&&) = delete;
    FFPlay& operator=(const FFPlay&) = delete;
    FFPlay& operator=(FFPlay&&) = delete;
};

using FFPlay_sptr = std::shared_ptr<FFPlay>;
FFPlay_sptr new_FFPlay() noexcept(false);

#endif
