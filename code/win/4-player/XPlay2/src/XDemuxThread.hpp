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
    /**
     * 打开媒体文件
     */
    virtual void Open(const QString &,IVideoCall *) noexcept(false);
    /**
     * 手动销毁本对象
     */
    virtual void Close() noexcept(false);
    /**
     * 启动线程
     */
    virtual void Start() noexcept(false);
    /**
     * 清除有关缓存
     */
    virtual void Clear() noexcept(true);
    /**
     * seek媒体文件
     */
    virtual void Seek(const double &) noexcept(false);

    /**
     * 获取媒体文件总时长
     * @return
     */
    [[nodiscard]] virtual int64_t totalMS() const noexcept(true){
        return m_total_Ms;
    }

    /**
     * 获取音频/ 视频的PTS,用于对外作用,比如用于进度条的显示
     * @return
     */
    [[nodiscard]] virtual int64_t Pts() const noexcept(true) {
        return m_pts;
    }

    /**
     * 暂停设置
     * @param b
     */
    virtual void SetPause(const bool &b);

    /**
     * 获取当前是否为暂停状态
     * @return
     */
    [[nodiscard]] virtual bool is_Pause() const noexcept(true){
        return m_isPause;
    }

    virtual void SetVolume(const double &) noexcept(true);
    [[nodiscard]] virtual double Volume() const noexcept(true);

    virtual void SetSpeed(const float &speed) noexcept(true);
    [[nodiscard]] float Speed() const noexcept(true);

protected:
    std::atomic_int64_t m_pts{},m_total_Ms{};
    std::atomic<std::exception_ptr*> m_ex_ptr{};
    std::atomic_bool m_is_Exit{},m_isPause{};
    QMutex m_mux;
    QWaitCondition m_cv;
    QSharedPointer<XDemux> m_demux;
    //std::atomic<float> m_speed{};
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
