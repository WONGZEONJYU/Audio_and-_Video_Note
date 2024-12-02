#ifndef XTOOLS_HPP_
#define XTOOLS_HPP_

#include "xhelper.hpp"

class XLIB_API XThread {
    virtual void Main() = 0;
    void _stop_();
    void _wait_();
public:
    /**
     * 休眠
     * @param ms
     */
    static void MSleep(const uint64_t &ms);

    /**
     * 启动线程
     */
    virtual void Start();

    /**
     * 停止线程
     */
    virtual void Stop();

    /**
     * 回收线程
     */
    virtual void Wait();
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

    virtual void pause(const bool &b){
        m_is_pause_ = b;
    }

    inline auto is_pause() const{
        return m_is_pause_.load(std::memory_order_relaxed);
    }

    inline auto is_exit() const{
        return m_is_exit_.load(std::memory_order_relaxed);
    }

private:
    std::atomic_bool m_is_exit_{true},
        m_is_pause_{};
    std::mutex m_mux_;
    std::thread m_th_;
    std::atomic_int m_index_{};
    std::atomic<XThread*> m_next_;

protected:
    explicit XThread() = default;
    virtual ~XThread();
    X_DISABLE_COPY_MOVE(XThread)
};

class XLIB_API XAVPacketList final {
    static inline constexpr auto max_packets{1000};
public:
    [[nodiscard]] XAVPacket_sp Pop();
    void Push(XAVPacket_sp &&);
    void Push(const XAVPacket_sp &);
    [[nodiscard]] bool Push(const XAVPacket &);
    [[nodiscard]] [[maybe_unused]] bool Push(XAVPacket &&);
    [[nodiscard]] uint64_t Size() const;
    void Clear();
private:
    std::mutex m_mux_;
    std::list<XAVPacket_sp> m_packets_;
public:
    explicit XAVPacketList() = default;
    ~XAVPacketList() = default;
    X_DISABLE_COPY_MOVE(XAVPacketList)
};

#endif
