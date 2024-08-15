//
// Created by Administrator on 2024/8/9.
//

#ifndef XPLAY2_XDECODETHREAD_HPP
#define XPLAY2_XDECODETHREAD_HPP

#include <QThread>
#include <QMutex>
#include <QSharedPointer>
#include <QWaitCondition>
#include <QQueue>
#include <QReadWriteLock>

class XAVCodecParameters;
class XDecode;
class XAVPacket;
class XAVFrame;

using XAVCodecParameters_sptr = typename std::shared_ptr<XAVCodecParameters>;
using XAVPacket_sptr = typename std::shared_ptr<XAVPacket>;
using XAVFrame_sptr = typename std::shared_ptr<XAVFrame>;

class XDecodeThread : public QThread {
Q_OBJECT

    void run() noexcept(false) override;
    /**
     * 派生类需实现线程入口函数
     */
    virtual void entry() noexcept(false) = 0;

protected:
    static inline constexpr auto Max_List{100};
    explicit XDecodeThread(std::exception_ptr * = nullptr);
    ~XDecodeThread() override;

    /**
     * 派生类需实现打开函数
     */
    virtual void Open(const XAVCodecParameters_sptr &) noexcept(false);

    /**
     * 创建解码器
     */
    void Create_Decode() noexcept(false);

    /**
     * 读取一包未解码数据,如果读取不到则返回空,建议使用之前先调用Empty()进行判空
     * @return XAVPacket_sptr or nullptr
     */
    virtual XAVPacket_sptr Pop() noexcept(false);

    /**
     * 配合Pop()使用.移除队列的帧
     */
    virtual void PopFront() noexcept(false);

    /**
     * 判空
     * @return true or false
     */
    virtual bool Empty() noexcept(false);

    /**
     * 发送解码包
     */
    virtual bool Send_Packet(const XAVPacket_sptr &) noexcept(false);

    virtual bool Send_Packet() noexcept(false);

    [[nodiscard]] virtual XAVFrame_sptr Receive_Frame(int64_t &pts) noexcept(false);

public:

    /**
     * 清空相关缓存
     */
    virtual void Clear() noexcept(true);

    /**
     * 关闭解码线程
     */
    virtual void Close() noexcept(true);
    /**
     * 线程退出等待,派生类可无需写等待线程退出
     */
    void Exit_Thread() noexcept(true);

    /**
     * 设置异常指针,用于异步处理异常
     * @param e
     */
    void SetException_ptr(std::exception_ptr *e) noexcept(true){
        m_exceptionPtr = e;
    }

    /**
     * 插入一包未解码数据,失败则等待
     */
    virtual void Push(XAVPacket_sptr &&) noexcept(false);

    void Set_Sync_Pts(const int64_t &pts) noexcept(true){
        m_sync_pts = pts;
    }

    [[nodiscard]] int64_t Pts() const noexcept(true) {return m_pts;}

    virtual void SetPause(const bool &b) noexcept(true){
        m_is_Pause = b;
    }

//    [[nodiscard]] auto is_Pause() const noexcept(true){
//        return m_is_Pause.load();
//    }

protected:
    std::atomic<std::exception_ptr *> m_exceptionPtr{};
    std::atomic_bool m_is_Exit{},m_is_Pause{};
    std::atomic_int64_t m_pts{},m_sync_pts{};
private:
    QQueue<XAVPacket_sptr> m_Packets;
    QWaitCondition m_cv;
    QMutex m_d_mux;
    QReadWriteLock m_rw_mux;
    QSharedPointer<XDecode> m_decode;
};

#endif
