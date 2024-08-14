//
// Created by wong on 2024/8/10.
//

#ifndef XPLAY2_XDEMUXTHREAD_HPP
#define XPLAY2_XDEMUXTHREAD_HPP

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QSharedPointer>

class QString;
class XDemux;
class XAudioThread;
class XVideoThread;
class IVideoCall;
class XAVCodecParameters;

using XAVCodecParameters_sptr = typename std::shared_ptr<XAVCodecParameters>;

class XDemuxThread : public QThread {
Q_OBJECT

    void run() override;
    void DeConstruct() noexcept(true);
public:
    explicit XDemuxThread(std::exception_ptr * = nullptr);
    virtual void Open(const QString &,IVideoCall *) noexcept(false);
    virtual void Close() noexcept(false);
    virtual void Start() noexcept(false);

    [[nodiscard]] auto totalMS() const noexcept(true){
        return m_total_Ms.load();
    }

    [[nodiscard]] auto Pts() const noexcept(true){
        return m_pts.load();
    }

    void SetPause(const bool &b){
        m_isPause = b;
    };

    [[nodiscard]] auto is_Pause() const noexcept(true){
        return m_isPause.load();
    }

protected:
    std::atomic_int64_t m_pts{},m_total_Ms{};
    std::atomic<std::exception_ptr*> m_ex_ptr{};
    std::atomic_bool m_is_exit{},m_isPause{};
    QMutex m_mux;
    QWaitCondition m_cv;
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
