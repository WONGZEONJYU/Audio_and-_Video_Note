//
// Created by wong on 2024/8/8.
//

#ifndef XPLAY2_XAUDIOTHREAD_HPP
#define XPLAY2_XAUDIOTHREAD_HPP

#include <memory>
#include <atomic>
#include <QThread>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QSharedPointer>
#include <QList>
#include "XHelper.hpp"

class XAVCodecParameters;
class XDecode;
class XResample;
class XAudioPlay;
class XAVPacket;

using XAVCodecParameters_sptr = typename std::shared_ptr<XAVCodecParameters>;
using XAVPacket_sptr = std::shared_ptr<XAVPacket>;

class XAudioThread : public QThread {
    static inline constexpr auto Max_List{100};
Q_OBJECT
    void DeConstruct() noexcept(true);
    void run() noexcept(false) override;
public:
    explicit XAudioThread(std::exception_ptr * = nullptr);
    virtual void Open(const XAVCodecParameters_sptr &) noexcept(false);
    //virtual void Close() noexcept(true);
    virtual void Push(XAVPacket_sptr &&) ;

protected:
    std::exception_ptr *m_exceptionPtr{};
    QRecursiveMutex m_re_mux;
    QWaitCondition m_wc;
    std::atomic_bool m_is_Exit;
    QList<XAVPacket_sptr> m_Packets;
    QSharedPointer<XDecode> m_decode;
    QSharedPointer<XResample> m_resample;
    XAudioPlay *m_audio_play{};

public:
    ~XAudioThread() override;
    X_DISABLE_COPY_MOVE(XAudioThread)
};

#endif
