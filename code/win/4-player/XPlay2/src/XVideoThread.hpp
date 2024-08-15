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
    void Open(const XAVCodecParameters_sptr &,IVideoCall *) noexcept(false);
protected:
    std::atomic<IVideoCall*> m_call{};
    //QMutex m_v_mux;
public:
    ~XVideoThread() override;
};

#endif
