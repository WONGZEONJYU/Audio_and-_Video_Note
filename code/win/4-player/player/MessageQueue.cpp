//
// Created by Administrator on 2024/6/25.
//

#include "MessageQueue.hpp"

MessageQueue::MessageQueue() noexcept(true):m_abort_request(true)  {

}

void MessageQueue::flush() noexcept(true) {

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
