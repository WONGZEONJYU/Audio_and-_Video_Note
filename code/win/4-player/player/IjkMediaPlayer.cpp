//
// Created by Administrator on 2024/6/27.
//

#include "IjkMediaPlayer.hpp"
IjkMediaPlayer::IjkMediaPlayer(const MessageAbstract &m):m_msg_loop{m} {

}

IjkMediaPlayer::~IjkMediaPlayer() {
    m_msg_thread.join();
}

void IjkMediaPlayer::start() {
    std::unique_lock<std::mutex> lock(m_mux);

}

void IjkMediaPlayer::stop() {
    std::unique_lock<std::mutex> lock(m_mux);

}
