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

class XDemux;
class XAudioThread;
class XVideoThread;
class IVideoCall;
class XAVCodecParameters;
using XAVCodecParameters_sptr = std::shared_ptr<XAVCodecParameters>;

class XDemuxThread : public QThread {
Q_OBJECT

    void run() override;
    void DeConstruct() noexcept(true);
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
    /**
     * 音频解码线程
     */
    QSharedPointer<XAudioThread> m_at;
    /**
     * 视频解码线程
     */
    QSharedPointer<XVideoThread> m_vt;
    /**
     * 音频解码信息
     */
    XAVCodecParameters_sptr m_ac;
    /**
     * 视频解码信息
     */
    XAVCodecParameters_sptr m_vc;

public:
    ~XDemuxThread() override;
};

#endif
