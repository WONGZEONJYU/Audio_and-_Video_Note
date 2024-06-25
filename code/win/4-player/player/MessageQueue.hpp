//
// Created by Administrator on 2024/6/25.
//

#ifndef PLAYER_MESSAGEQUEUE_HPP
#define PLAYER_MESSAGEQUEUE_HPP

#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>

struct AVMessage{
    int what;
};

class MessageQueue final{

public:
    MessageQueue(const MessageQueue&) = delete;
    MessageQueue(MessageQueue&&) = delete;
    MessageQueue& operator=(const MessageQueue&) = delete;
    MessageQueue& operator=(MessageQueue&&) = delete;

    explicit MessageQueue() noexcept(true);
    ~MessageQueue();
    void flush() noexcept(true);
    void abort() noexcept(true);
    void start() noexcept(true);

private:
    std::atomic_bool m_abort_request;
    std::mutex m_mux;
    std::condition_variable m_cv;
    std::queue<AVMessage> m_msg_q;

};


#endif
