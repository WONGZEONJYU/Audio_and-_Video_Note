//
// Created by Administrator on 2024/6/27.
//

#include "IjkMediaPlayer.hpp"
#include "IjkMediaPlayer_State.h"
#include "ff_ffmsg.h"
#include <iostream>

IjkMediaPlayer::IjkMediaPlayer(MessageAbstract<IjkMediaPlayer*> &m):m_msg_loop{m} {

}

void IjkMediaPlayer::construct() noexcept(false) {
    m_ff = new_FFPlay();
}

IjkMediaPlayer::~IjkMediaPlayer() {
    std::cerr << __FUNCTION__ << "\n";
    if (m_msg_thread.joinable()){
        m_msg_thread.join();
    }
}

void IjkMediaPlayer::set_data_source(std::string &&url) noexcept(false) {
    if (url.empty()){
        throw std::runtime_error("IjkMediaPlayer::set_data_source url empty\n");
    }
    m_data_source = std::move(url);
}

void IjkMediaPlayer::prepare_async() noexcept(false)
{
    std::unique_lock<std::mutex> lock(m_mux);
    try {
        m_mp_state = MP_STATE_ASYNC_PREPARING;

        m_ff->mq_start(); //队列启动

        m_msg_thread = std::thread([this]() {
            m_msg_loop.msg_loop(this);
        });

        m_ff->prepare_async(m_data_source);

    } catch (const std::runtime_error &e) {
        m_mp_state = MP_STATE_ERROR;
        lock.unlock();
        throw e;
    } catch (const std::exception &e) {
        lock.unlock();
        throw e;
    }
}

void IjkMediaPlayer::start() {
    std::unique_lock<std::mutex> lock(m_mux);
    m_ff->mq_msg_put(FFP_REQ_START);
}

void IjkMediaPlayer::stop() {

    std::unique_lock<std::mutex> lock(m_mux);
    m_ff->f_stop();
}

int IjkMediaPlayer::get_msg(AVMessage_Sptr &msg,const bool &block)
{
    while (true){
        bool is_next_msg{}; //有些消息内部处理
        auto ret {m_ff->mq_msg_get(msg, block)};
        if (ret <= 0){
            return ret;
        }
        const auto what{msg->what()};
        switch (what) {
            case FFP_MSG_PREPARED:
                std::cerr << __FUNCTION__  << "\tFFP_MSG_PREPARED\n";
                break;
            case FFP_REQ_START:
                std::cerr << __FUNCTION__ << "\tFFP_REQ_START\n";
                is_next_msg = true;
                //m_ff->f_start();
                break;
            default:
                std::cerr << __FUNCTION__ << "\tother_msg:\t" << what << "\n";
                break;
        }

        if (!is_next_msg){ //不需内部处理的消息让UI那边处理
            return ret;
        }
    }
}

IjkMediaPlayer_sptr new_IjkMediaPlayer(MessageAbstract<IjkMediaPlayer*> &msg_loop) noexcept(false)
{
    IjkMediaPlayer_sptr obj;
    try {
        obj.reset(new IjkMediaPlayer(msg_loop));
        obj->construct();
        return obj;
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw e;
    } catch (const std::bad_alloc &e) {
        obj.reset();
        throw e;
    }
}