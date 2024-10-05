//
// Created by wong on 2024/9/26.
//

#include "xtools.hpp"
#include <sstream>

void XThread::_stop_() {

    m_is_exit = true;
    if (m_th.joinable()) {
        LOGDINFO(GET_STR(thread begin stop));
        m_th.join();
        std::string s(GET_STR(thread end stop :));
        s += std::to_string(m_index);
        LOGDINFO(s);
    }
}

void XThread::Start() {
    static std::atomic_int i{};
    m_is_exit = false;
    std::unique_lock locker(m_mux);
    m_th = std::thread(&XThread::Main, this);
    m_index.store(i++);
    std::stringstream ss;
    ss << GET_STR(thread start index:) << m_index << " ";
    LOGDINFO(ss.str());
}

void XThread::Stop() {
    _stop_();
}

XThread::~XThread() {
    _stop_();
}


