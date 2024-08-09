//
// Created by wong on 2024/8/10.
//

#ifndef XPLAY2_XDEMUXTHREAD_HPP
#define XPLAY2_XDEMUXTHREAD_HPP

#include <QThread>
#include <QRecursiveMutex>
#include <QSharedPointer>

class XDemux;
class XAudioThread;
class XVideoThread;

class XDemuxThread : public QThread {
Q_OBJECT

    void run() override;
public:
    explicit XDemuxThread(std::exception_ptr * = nullptr);
    virtual void Open() noexcept(false);

protected:
    std::atomic<std::exception_ptr*> m_ex_ptr{};
    std::atomic_bool m_is_exit{};
    QRecursiveMutex m_re_mux;
    QSharedPointer<XDemux> m_demux;

public:
    ~XDemuxThread() override;

};


#endif //XPLAY2_XDEMUXTHREAD_HPP
