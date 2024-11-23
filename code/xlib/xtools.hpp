#ifndef XTOOLS_HPP_
#define XTOOLS_HPP_

#include "xhelper.hpp"

XLIB_API auto XRescale(const int64_t &pts,
    const AVRational &src_tb,
    const AVRational &dst_tb) -> int64_t;

XLIB_API auto XRescale(const int64_t &pts,
    const XRational &src_tb,
    const XRational &dst_tb) ->int64_t;

class XLIB_API XThread {
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
        m_next_ = xt;
    }

    /**
     * 传递到下一个责任链函数
     * @param pkt
     */
    virtual void Next(XAVPacket &pkt) {
        if (m_next_){
            m_next_.load()->Do(pkt);
        }
    }

    /**
     * 执行任务,用户如有需要则自行重写该函数
     * @param XAVPacket &
     */
    virtual void Do(XAVPacket &){}

protected:
    std::atomic_bool m_is_exit_{};

private:
    std::mutex m_mux_;
    std::thread m_th_;
    std::atomic_int m_index_{};
    std::atomic<XThread*> m_next_;

protected:
    explicit XThread() = default;
    virtual ~XThread();
    X_DISABLE_COPY_MOVE(XThread)
};

class XLIB_API XAVPacketList {
    static inline constexpr auto max_packets{1000};
public:
    [[nodiscard]] XAVPacket_sp Pop();
    void Push(XAVPacket_sp &&);
    void Push(const XAVPacket_sp &);
    [[nodiscard]] bool Push(const XAVPacket &);
    [[nodiscard]] bool Push(XAVPacket &&);
    [[nodiscard]] uint64_t Size() const;
private:
    std::mutex m_mux_;
    std::list<XAVPacket_sp> m_packets_;
public:
    explicit XAVPacketList() = default;
    X_DISABLE_COPY_MOVE(XAVPacketList)
};

#endif
