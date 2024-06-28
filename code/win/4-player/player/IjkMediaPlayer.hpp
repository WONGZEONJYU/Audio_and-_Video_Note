//
// Created by Administrator on 2024/6/27.
//

#ifndef PLAYER_IJKMEDIAPLAYER_HPP
#define PLAYER_IJKMEDIAPLAYER_HPP

#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <string>
#include <thread>
#include <vector>
#include "MessageAbstract.hpp"
#include "FFPlay.hpp"
#include "IjkMediaPlayer_State.h"

class IjkMediaPlayer final {

public:
    explicit IjkMediaPlayer(MessageAbstract<IjkMediaPlayer*> &);
    ~IjkMediaPlayer();

    void start();
    void stop();
    void set_data_source(std::string &&) noexcept(false);
    void prepare_async();

private:
    MessageAbstract<IjkMediaPlayer*> &m_msg_loop;
    std::thread m_msg_thread;
    std::mutex m_mux;
    std::string m_data_source;
    FFPlay m_ff;
    int m_mp_state{};

public:
    IjkMediaPlayer(const IjkMediaPlayer&) = delete;
    IjkMediaPlayer& operator=(const IjkMediaPlayer&) = delete;
};

#endif
