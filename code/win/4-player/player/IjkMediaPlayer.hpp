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

class IjkMediaPlayer final {

public:
    explicit IjkMediaPlayer() = default;
    ~IjkMediaPlayer();




private:
    std::atomic_int m_ref_count{};
    int m_mp_state{};
    std::string m_data_source;
    std::thread m_msg_loop_thread{};

public:
    IjkMediaPlayer(const IjkMediaPlayer&) = delete;
    IjkMediaPlayer& operator=(const IjkMediaPlayer&) = delete;
};


#endif //PLAYER_IJKMEDIAPLAYER_HPP
