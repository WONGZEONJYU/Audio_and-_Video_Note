//
// Created by Administrator on 2024/6/25.
//

#include "MessageQueue.hpp"
#include "ff_ffmsg.h"

//MessageQueue::MessageQueue() noexcept(true){
//
//}

void MessageQueue::flush() noexcept(true) {
    std::unique_lock<std::mutex> lock(m_mux);
    m_msg_q.clear();
}

//MessageQueue::~MessageQueue() {
//
//}

void MessageQueue::abort() noexcept(true) {
    m_abort_request = true;
    m_cv.notify_all();
}

void MessageQueue::start() noexcept(true) {
    std::unique_lock<std::mutex> lock(m_mux);
    put_helper(AVMessage{FFP_MSG_FLUSH});
    m_abort_request = false;
}

int MessageQueue::put_helper(AVMessage &&msg) noexcept(true) {

    if (m_abort_request){
        return -1;
    }

    m_msg_q.emplace_back(std::make_shared<AVMessage>(std::move(msg)));
    m_cv.notify_all();
    return 0;
}

int MessageQueue::msg_put(AVMessage &&msg) noexcept(true) {
    std::unique_lock<std::mutex> lock(m_mux);
    return put_helper(std::move(msg));
}

int MessageQueue::msg_put(const AVMessage &msg) noexcept(true)
{
    AVMessage msg1(msg);
    return msg_put(std::move(msg1));
}

int MessageQueue::msg_put(const int &msg) noexcept(true)
{
    AVMessage msg1(msg);
    return msg_put(std::move(msg1));
}

int MessageQueue::msg_get(AVMessage_Sptr& msg, const bool &is_block) noexcept(true) {

    auto ret_val{1};
    std::unique_lock<std::mutex> lock(m_mux);

    for (;;){

        if (m_abort_request){
            ret_val = -1;
            break;
        }

        if (!m_msg_q.empty()){
            msg = std::move(m_msg_q.front());
            m_msg_q.pop_front();
            break;
        } else if (!is_block){
            ret_val = 0;
            break;
        } else{
            m_cv.wait(lock);
        }
    }

    return ret_val;
}

void MessageQueue::remove(const int &what) noexcept(true) {

    std::unique_lock<std::mutex> lock(m_mux);

    if (!m_abort_request && !m_msg_q.empty()) {

        std::erase_if(m_msg_q,[&what](const auto &item){
            return what == item->m_what;
        });
    }
}
