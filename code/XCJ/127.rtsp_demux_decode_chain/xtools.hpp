//
// Created by wong on 2024/9/26.
//

#ifndef INC_126_RTSP_DEMUX_THREAD_XTOOLS_HPP
#define INC_126_RTSP_DEMUX_THREAD_XTOOLS_HPP

#include <thread>
#include <mutex>
#include "xhelper.hpp"

class XAVPacket;

class XThread {
    virtual void Main() = 0;
    void _stop_();
public:
    virtual void Start();
    virtual void Stop();

    /**
     * 设置责任链下一个节点(线程安全)
     * @param xt
     */
    void set_next(XThread *xt){
        //std::unique_lock locker(m_mux);
        m_next = xt;
    }

    /**
     * 传递到下一个责任链函数
     * @param pkt
     */
    virtual void Next(XAVPacket &pkt){
        //std::unique_lock locker(m_mux);
        if (m_next){
            m_next.load()->Do(pkt);
        }
    }

    /**
     * 执行任务
     * @param pkt
     */
    virtual void Do(XAVPacket &pkt){
        (void )pkt;
    };

protected:
    std::atomic_bool m_is_exit{};

private:
    std::thread m_th;
    std::mutex m_mux;
    std::atomic_int m_index{};
    //XThread *m_next{};
    std::atomic<XThread*> m_next;

protected:
    explicit XThread() = default;
    virtual ~XThread();
    X_DISABLE_COPY_MOVE(XThread)
};

class XTools {

};

#endif
