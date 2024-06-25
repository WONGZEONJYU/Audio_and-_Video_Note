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

    explicit AVMessage() = default;

    explicit AVMessage(const int& what):m_what{what}{
    }

    AVMessage(const AVMessage& obj){
        m_what = obj.m_what;
    }

    AVMessage(AVMessage&& obj) noexcept {
        m_what = obj.m_what;
        obj.m_what = 0;
    }

    AVMessage& operator=(const AVMessage& obj){
        if (this != &obj){
            m_what = obj.m_what;
        }
        return *this;
    }

    AVMessage& operator=(AVMessage&& obj) noexcept {
        if (this != &obj){
            m_what = obj.m_what;
            obj.m_what = 0;
        }
        return *this;
    }

    int m_what{};
};

using AVMessage_Sptr = std::shared_ptr<AVMessage>;

class MessageQueue final{

    int put_helper(AVMessage &&s) noexcept(true);

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
    int msg_put(AVMessage &&) noexcept(true);
    int msg_get(AVMessage_Sptr&,const bool &) noexcept(true);

private:
    std::atomic_bool m_abort_request;
    std::mutex m_mux;
    std::condition_variable m_cv;
    std::deque<AVMessage_Sptr> m_msg_q;
};

#endif
