//
// Created by wong on 2024/9/26.
//

#ifndef INC_126_RTSP_DEMUX_THREAD_XTOOLS_HPP
#define INC_126_RTSP_DEMUX_THREAD_XTOOLS_HPP

#include <thread>
#include <mutex>
#include "xhelper.hpp"

class XThread {
    virtual void Main() = 0;
    void _stop_();
public:
    virtual void Start();
    virtual void Stop();
protected:
    std::atomic_bool m_is_exit{};

private:
    std::thread m_th;
    std::mutex m_mux;
    std::atomic_int m_index{};
protected:
    explicit XThread() = default;
    virtual ~XThread();
    X_DISABLE_COPY_MOVE(XThread)
};

class XTools {

};

#endif
