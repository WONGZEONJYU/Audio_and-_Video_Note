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

class XAVCodecParameters;
class XDecode;
class XAVPacket;

using XAVCodecParameters_sptr = typename std::shared_ptr<XAVCodecParameters>;
using XAVPacket_sptr = std::shared_ptr<XAVPacket>;

class XDecodeThread : public QThread {
Q_OBJECT

    void run() noexcept(false) override;

protected:
    static inline constexpr auto Max_List{100};
    explicit XDecodeThread(std::exception_ptr * = nullptr);
    ~XDecodeThread() override;

    /**
     * 派生类需实现线程入口函数
     */
    virtual void entry() noexcept(false) = 0;

    /**
     * 派生类需实现打开函数
     */
    virtual void Open(const XAVCodecParameters_sptr &) = 0;

    /**
     * 创建解码器
     */
    void Create_Decode() noexcept(false);

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

    /**
     * 读取一包未解码数据,如果读取不到则返回空
     * @return XAVPacket_sptr or nullptr
     */

    virtual XAVPacket_sptr Pop() noexcept(false);

    /**
     * 配合Pop()使用.移除队列的帧
     */
    virtual void PopFront() noexcept(false);

    void Set_Sync_Pts(const int64_t &pts) {
        m_sync_pts = pts;
    }

    [[nodiscard]] int64_t Pts() const {return m_pts;}

protected:
    std::atomic<std::exception_ptr *> m_exceptionPtr{};
    std::atomic_bool m_is_Exit{};
    QSharedPointer<XDecode> m_decode;
    std::atomic_int64_t m_pts{},m_sync_pts{};
    QQueue<XAVPacket_sptr> m_Packets;
    //QWaitCondition m_cv;
    QMutex m_d_mux;
};

#endif
