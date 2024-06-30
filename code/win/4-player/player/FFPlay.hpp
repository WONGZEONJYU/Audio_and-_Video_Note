//
// Created by Administrator on 2024/6/27.
//

#ifndef PLAYER_FFPLAY_HPP
#define PLAYER_FFPLAY_HPP

#include <string>
#include <thread>
#include <atomic>
#include "MessageQueue.hpp"

class FFPlay final : public MessageQueue {

    explicit FFPlay() = default;
    void read_thread();
    void video_refresh_thread();
    void stream_open();
    void stream_component_open(const int&);
    void stream_component_close(const int&);

    friend class std::shared_ptr<FFPlay> new_FFPlay() noexcept(false);
public:
    ~FFPlay() override ;
    void prepare_async(const std::string &);
    void stream_close();
    void f_start();
    void f_stop();
private:
    std::string m_url;
    std::thread m_read_th;
    std::thread m_video_refresh_th;
    std::thread m_audio_th;
    std::atomic_bool abort_request{};
    int audio_stream {-1},video_stream{-1};

public:
    FFPlay(const FFPlay&) = delete;
    FFPlay(FFPlay&&) = delete;
    FFPlay& operator=(const FFPlay&) = delete;
    FFPlay& operator=(FFPlay&&) = delete;
};

using FFPlay_sptr = std::shared_ptr<FFPlay>;
FFPlay_sptr new_FFPlay() noexcept(false);

#endif
