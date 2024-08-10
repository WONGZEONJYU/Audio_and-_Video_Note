//
// Created by wong on 2024/8/10.
//

#ifndef XPLAY2_XDEMUXTHREAD_HPP
#define XPLAY2_XDEMUXTHREAD_HPP

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QSharedPointer>
#include <QString>
#include "IVideoCall.hpp"

class XDemux;
class XAudioThread;
class XVideoThread;

class XDemuxThread : public QThread {
Q_OBJECT

    void run() override;
public:
    explicit XDemuxThread(std::exception_ptr * = nullptr);
    virtual void Open(const QString &,IVideoCall *) noexcept(false);
    virtual void Start() noexcept(false);
protected:
    std::atomic<std::exception_ptr*> m_ex_ptr{};
    std::atomic_bool m_is_exit{};
    QMutex m_mux;
    QWaitCondition m_wc;
    QSharedPointer<XDemux> m_demux;
    QSharedPointer<XAudioThread> m_at;
    QSharedPointer<XVideoThread> m_vt;

public:
    ~XDemuxThread() override;

};


#endif //XPLAY2_XDEMUXTHREAD_HPP
