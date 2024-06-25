//
// Created by Administrator on 2024/6/25.
//

#include "MessageQueue.hpp"

MessageQueue::MessageQueue() noexcept(true):m_abort_request(true)  {

}

void MessageQueue::flush() noexcept(true) {
    m_msg_q.clear();
}

MessageQueue::~MessageQueue() {
    flush();
}

void MessageQueue::abort() noexcept(true) {
    m_abort_request = true;
    m_cv.notify_all();
}

void MessageQueue::start() noexcept(true) {
    m_abort_request = false;
}

int MessageQueue::put_helper(AVMessage &&msg) noexcept(true) {

    if (m_abort_request){
        return -1;
    }

    auto av_msg{std::make_shared<AVMessage>(std::move(msg))};
    m_msg_q.push_back(std::move(av_msg));
    m_cv.notify_all();
    return 0;
}

int MessageQueue::msg_put(AVMessage &&msg) noexcept(true) {
    std::unique_lock<std::mutex> lock(m_mux);
    return put_helper(std::move(msg));
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



