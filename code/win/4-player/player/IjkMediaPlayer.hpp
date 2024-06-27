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

class IjkMediaPlayer final {

public:
    explicit IjkMediaPlayer(const MessageAbstract &);
    ~IjkMediaPlayer();

    void start();
    void stop();

private:
    const MessageAbstract &m_msg_loop;
    std::thread m_msg_thread;
    std::mutex m_mux;
    std::string m_data_source;
    int m_mp_state{};

public:
    IjkMediaPlayer(const IjkMediaPlayer&) = delete;
    IjkMediaPlayer& operator=(const IjkMediaPlayer&) = delete;
};

#endif //PLAYER_IJKMEDIAPLAYER_HPP
