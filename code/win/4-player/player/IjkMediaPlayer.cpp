//
// Created by Administrator on 2024/6/27.
//

#include "IjkMediaPlayer.hpp"
IjkMediaPlayer::IjkMediaPlayer(MessageAbstract<IjkMediaPlayer*> &m):m_msg_loop{m} {

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

void IjkMediaPlayer::set_data_source(std::string &&url) noexcept(false) {
    if (url.empty()){
        throw std::runtime_error("url empty\n");
    }
    m_data_source = std::move(url);
}

void IjkMediaPlayer::prepare_async()
{
    std::unique_lock<std::mutex> lock(m_mux);
    try {
        m_mp_state = MP_STATE_ASYNC_PREPARING;

        m_ff.start();

        m_msg_thread = std::thread([this]() {
            m_msg_loop.msg_loop(this);
        });

        m_ff.prepare_async(m_data_source);
    } catch (const std::runtime_error &e) {
        m_mp_state = MP_STATE_ERROR;
        lock.unlock();
        throw e;
    } catch (const std::exception &e) {
        lock.unlock();
        throw e;
    }
}
