//
// Created by Administrator on 2024/6/25.
//

#include "MessageQueue.hpp"
#include "ff_ffmsg.h"
#include <algorithm>

void MessageQueue::mq_flush() noexcept(true) {
    std::unique_lock<std::mutex> lock(m_mux);
    m_msg_q.clear();
}

void MessageQueue::mq_abort() noexcept(true) {
    m_abort_request = true;
    m_cv.notify_all();
}

void MessageQueue::mq_start() noexcept(true) {
    std::unique_lock<std::mutex> lock(m_mux);
    mq_put_helper(AVMessage{FFP_MSG_FLUSH});
    m_abort_request = false;
}

int MessageQueue::mq_put_helper(AVMessage &&msg) noexcept(true) {

    if (m_abort_request){
        return -1;
    }

    m_msg_q.emplace_back(std::make_shared<AVMessage>(std::move(msg)));
    m_cv.notify_all();
    return 0;
}

int MessageQueue::mq_msg_put(AVMessage &&msg) noexcept(true) {
    std::unique_lock<std::mutex> lock(m_mux);
    return mq_put_helper(std::move(msg));
}

int MessageQueue::mq_msg_put(const AVMessage &msg) noexcept(true)
{
    AVMessage msg1(msg.m_what,msg.m_arg1,msg.m_arg2,msg.m_obj);
    return mq_msg_put(std::move(msg1));
}

int MessageQueue::mq_msg_put(const int &msg,
                          const int &arg1,
                          const int &arg2,
                          const char *obj,
                          const size_t &obj_len) noexcept(false)
{
    auto obj1{const_cast<char *>(obj)};
    if (obj && obj_len){
        obj1 = new char[obj_len]{};
        std::copy_n(obj,obj_len,obj1);
    }
    AVMessage msg1(msg,arg1,arg2,obj1);
    return mq_msg_put(std::move(msg1));
}

int MessageQueue::mq_msg_get(AVMessage_Sptr& msg, const bool &is_block) noexcept(true) {

    auto ret_val{1};
    std::unique_lock<std::mutex> lock(m_mux);

    while (true){

        if (m_abort_request) {
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

void MessageQueue::mq_remove(const int &what) noexcept(true) {

    std::unique_lock<std::mutex> lock(m_mux);

    if (!m_abort_request && !m_msg_q.empty()) {

        std::erase_if(m_msg_q,[&what](const auto &item){
            return what == item->m_what;
        });
    }
}

AVMessage::AVMessage(const int &what,const int &arg1 ,const int &arg2
        ,const char * const obj):
        m_what{what},m_arg1{arg1},m_arg2{arg2},m_obj{obj}{
}

AVMessage::AVMessage(AVMessage&& obj) noexcept {
    move_(std::move(obj));
}

AVMessage& AVMessage::operator=(AVMessage&& obj) noexcept
{
    if (this != &obj){
        move_(std::move(obj));
    }
    return *this;
}

void AVMessage::move_(AVMessage &&obj) {
    m_what = obj.m_what;
    m_arg1 = obj.m_arg1;
    m_arg2 = obj.m_arg2;
    m_obj = obj.m_obj;

    obj.m_obj = nullptr;
    obj.m_arg2 = obj.m_arg1 = obj.m_what = 0;
}

AVMessage::~AVMessage(){
    delete[] m_obj;
}
