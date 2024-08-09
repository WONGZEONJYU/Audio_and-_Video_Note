//
// Created by Administrator on 2024/8/9.
//

#ifndef XPLAY2_XAVQTHREAD_ABSTRACT_HPP
#define XPLAY2_XAVQTHREAD_ABSTRACT_HPP

#include <QThread>
#include <QRecursiveMutex>
#include <QSharedPointer>
#include <QWaitCondition>
#include <QQueue>

class XAVCodecParameters;
class XDecode;
class XAVPacket;

using XAVCodecParameters_sptr = typename std::shared_ptr<XAVCodecParameters>;
using XAVPacket_sptr = std::shared_ptr<XAVPacket>;

class XAVQThread_Abstract : public QThread {
Q_OBJECT

protected:
    static inline constexpr auto Max_List{100};
    explicit XAVQThread_Abstract(std::exception_ptr * = nullptr);
    ~XAVQThread_Abstract() override;
    void Exit_Thread() noexcept(true);
    void SetException_ptr(std::exception_ptr *) noexcept(true);
    virtual void Open(const XAVCodecParameters_sptr &) = 0;
    virtual void Push(XAVPacket_sptr &&) noexcept(false);

protected:
    std::atomic<std::exception_ptr *> m_exceptionPtr{};
    QRecursiveMutex m_re_mux;
    QWaitCondition m_wc;
    std::atomic_bool m_is_Exit{};
    QQueue<XAVPacket_sptr> m_Packets;
    QSharedPointer<XDecode> m_decode;
};

#endif
