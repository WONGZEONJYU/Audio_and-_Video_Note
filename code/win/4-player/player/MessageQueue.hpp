//
// Created by Administrator on 2024/6/25.
//

#ifndef PLAYER_MESSAGEQUEUE_HPP
#define PLAYER_MESSAGEQUEUE_HPP

#include <atomic>
#include <shared_mutex>
#include <queue>
#include <condition_variable>
#include <iostream>
#include <string>

struct AVMessage final {
    AVMessage(const AVMessage& obj) = delete;
    AVMessage& operator=(const AVMessage& obj) = delete;

    explicit AVMessage() = default;
    explicit AVMessage(const int &what,const int &arg1 = 0,const int &arg2 = 0
                       ,const char *obj = nullptr);

    AVMessage(AVMessage&& obj) noexcept;
    AVMessage& operator=(AVMessage&& obj) noexcept;

    ~AVMessage();

    [[nodiscard]] auto what() const{return m_what;}
    [[nodiscard]] auto arg1() const {return m_arg1;}
    [[nodiscard]] auto arg2() const {return m_arg2;}
    [[nodiscard]] auto obj() const{return m_obj;}

private:
    int m_what{},m_arg1{},m_arg2{};
    const char * m_obj{};

    void move_(AVMessage &&);
    friend class MessageQueue;
};

using AVMessage_Sptr = std::shared_ptr<AVMessage>;

class MessageQueue {

    int put_helper(AVMessage &&) noexcept(true);

public:
    explicit MessageQueue() noexcept(true) = default;
    virtual ~MessageQueue() = default;

    void flush() noexcept(true);
    void abort() noexcept(true);
    void start() noexcept(true);
    int msg_put(AVMessage &&) noexcept(true);
    int msg_put(const AVMessage &) noexcept(true);
    int msg_put(const int &what,
                const int &arg1 = 0,
                const int &arg2 = 0,
                const char *obj = nullptr,
                const size_t &obj_len = 0) noexcept(false);

    int msg_get(AVMessage_Sptr& ,const bool & = false) noexcept(true);
    void remove(const int &) noexcept(true);

private:
    std::atomic_bool m_abort_request{true};
    std::mutex m_mux;
    std::condition_variable_any m_cv;
    std::deque<AVMessage_Sptr> m_msg_q;

public:
    MessageQueue(const MessageQueue&) = delete;
    MessageQueue(MessageQueue&&) = delete;
    MessageQueue& operator=(const MessageQueue&) = delete;
    MessageQueue& operator=(MessageQueue&&) = delete;
};

#endif
