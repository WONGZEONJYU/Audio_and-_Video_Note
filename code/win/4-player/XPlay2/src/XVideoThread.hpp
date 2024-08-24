//
// Created by Administrator on 2024/8/9.
//

#ifndef XPLAY2_XVIDEOTHREAD_HPP
#define XPLAY2_XVIDEOTHREAD_HPP

#include "XDecodeThread.hpp"
#include "IVideoCall.hpp"

class XVideoThread : public XDecodeThread {
Q_OBJECT
    void entry() override;
public:
    explicit XVideoThread(std::exception_ptr * = nullptr);
    virtual void Open(const XAVCodecParameters_sptr &,IVideoCall *) noexcept(false);
    void Close() noexcept(true) override;
    /**
     * seek后显示到关键帧的作用
     */
    virtual bool RepaintPts(const XAVPacket_sptr &,const int64_t &) noexcept(false);
    void SetPause(const bool &) noexcept(true) override;
    virtual void HasAudio(const bool &b) noexcept(true){
        m_has_audio = b;
    }
protected:
    std::atomic<IVideoCall*> m_call{};
    QMutex m_v_mux;
    std::atomic_bool m_has_audio{true};
    //std::atomic_int64_t m_last_pts{};
    //QWaitCondition m_v_cv;
public:
    ~XVideoThread() override;
};

#endif
