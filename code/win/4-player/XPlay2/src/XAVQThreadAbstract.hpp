//
// Created by Administrator on 2024/8/9.
//

#ifndef XPLAY2_XAVQTHREADABSTRACT_HPP
#define XPLAY2_XAVQTHREADABSTRACT_HPP

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

class XAVQThreadAbstract : public QThread {
Q_OBJECT

protected:
    static inline constexpr auto Max_List{100};
    explicit XAVQThreadAbstract(std::exception_ptr * = nullptr);
    ~XAVQThreadAbstract() override;
    /**
     * 线程退出等待
     */
    void Exit_Thread() noexcept(true);
    //派生类可以直接
    //using XAVQThreadAbstract::SetException_ptr;
    void SetException_ptr(std::exception_ptr *) noexcept(true);
    virtual void Open(const XAVCodecParameters_sptr &) = 0;
    //派生类可以直接using XAVQThreadAbstract::Push;
    virtual void Push(XAVPacket_sptr &&) noexcept(false);

    std::atomic<std::exception_ptr *> m_exceptionPtr{};
    QMutex m_mux;
    QWaitCondition m_wc;
    std::atomic_bool m_is_Exit{};
    QQueue<XAVPacket_sptr> m_Packets;
    QSharedPointer<XDecode> m_decode;
};

#endif
