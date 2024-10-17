#ifndef INC_126_RTSP_DEMUX_THREAD_XTOOLS_HPP
#define INC_126_RTSP_DEMUX_THREAD_XTOOLS_HPP

#include <thread>
#include "xhelper.hpp"

class XTools {

};

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
        m_next_ = xt;
    }

    /**
     * 传递到下一个责任链函数
     * @param pkt
     */
    virtual void Next(XAVPacket &pkt) {
        //std::unique_lock locker(m_mux);
        if (m_next_){
            m_next_.load()->Do(pkt);
        }
    }

    /**
     * 执行任务,用户如有需要则自行继承
     * @param XAVPacket &
     */
    virtual void Do(XAVPacket &){}

protected:
    std::atomic_bool m_is_exit_{};

private:
    std::thread m_th_;
    std::mutex m_mux_;
    std::atomic_int m_index_{};
    //XThread *m_next{};
    std::atomic<XThread*> m_next_;

protected:
    explicit XThread() = default;
    virtual ~XThread();
    X_DISABLE_COPY_MOVE(XThread)
};

class XAVPacketList {
    static inline constexpr auto max_packets{100};
public:
    XAVPacket_sp Pop();
    void Push(XAVPacket_sp &&);
    void Push(XAVPacket_sp &);
    void Push(XAVPacket &);
    void Push(XAVPacket &&);
private:
    std::list<XAVPacket_sp> m_packets_;
    std::mutex m_mux_;
public:
    explicit XAVPacketList() = default;
    X_DISABLE_COPY_MOVE(XAVPacketList)
};

#endif
