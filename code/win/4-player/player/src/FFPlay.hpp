//
// Created by Administrator on 2024/6/27.
//

#ifndef PLAYER_FFPLAY_HPP
#define PLAYER_FFPLAY_HPP

#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include "MessageQueue.hpp"
#include "ff_ffplay_def.hpp"
#include "VideoDecoder.hpp"
#include "AudioDecoder.hpp"
#include "SwrContext_t.hpp"
#include <Sws_Context.hpp>
#include <QImage>

class FFPlay final : protected MessageQueue {

    friend class std::shared_ptr<FFPlay> new_FFPlay() noexcept(false);
    static int decode_interrupt_cb(void *);
    static void sdl_audio_callback(void *, Uint8 *, int);
    explicit FFPlay() = default;
    void construct() noexcept(false);
    void read_thread();
    void video_refresh_thread();
    void stream_open() noexcept(false);
    void stream_close() noexcept(true);
    void stream_component_open(const int&) noexcept(false);
    void stream_component_close(const int&);
    uint32_t audio_open(const AVChannelLayout &,const int &,const int &,AudioParams &) noexcept(false);
    int audio_decode_frame() noexcept(true);
    void video_refresh(double &);
    double get_master_clock();
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

    void Add_VideoRefreshCallback(auto &&f)  noexcept(true){
        if (!m_video_refresh_callback){
            m_video_refresh_callback = std::forward<decltype(f)>(f);
        }
    }

private:
    std::string m_url;
    std::thread m_read_th,m_video_refresh_th;
    std::condition_variable_any m_cv;
    std::atomic_bool m_abort_request{},
                    m_eof{},
                    m_muted{};

    int m_audio_stream {-1},m_video_stream{-1},
    m_av_sync_type{AV_SYNC_AUDIO_MASTER},
    m_startup_volume{100},
    m_audio_volume{};

    double m_audio_clock{};             // 当前音频帧的PTS+当前帧Duration

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

    VideoDecoder_sptr m_v_decoder{};
    AudioDecoder_sptr m_a_decoder{};

    AudioParams m_audio_src{}, //音频解码后到frame参数
                m_audio_tgt{}; //SDL支持到音频参数,重采样转换,m_audio_src->m_audio_tgt
    SwrContext_sp_type m_swr_ctx{}; //重采样器
    Sws_Context_sptr m_sws_ctx{};
    int m_audio_hw_size{}, //SDL音频缓冲区的大小(字节为单位)
        m_audio_buf_index{}; //用于更新SDL回调函数拷贝位置,当前音频帧中已拷入SDL音频缓冲区
    uint8_t *m_audio_buf{}, //指向待重采样待数据
            *m_audio_buf1{}; //指向重采样后的数据
    uint32_t m_audio_buf_size{}, //m_audio_buf指向的内存大小(待播放待一帧音频数据的大小)
            m_audio_buf1_size{}; //m_audio_buf1指向的内存大小(申请到的音频缓冲区)

   std::function<int(QImage&&)> m_video_refresh_callback;

   //uint8_t *m_video_dst_buf[4]{};
   //uint32_t m_video_dst_size{};

public:
    FFPlay(const FFPlay&) = delete;
    FFPlay(FFPlay&&) = delete;
    FFPlay& operator=(const FFPlay&) = delete;
    FFPlay& operator=(FFPlay&&) = delete;
};

using FFPlay_sptr = std::shared_ptr<FFPlay>;
FFPlay_sptr new_FFPlay() noexcept(false);

#endif
