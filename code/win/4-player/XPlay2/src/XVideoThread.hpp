//
// Created by Administrator on 2024/8/9.
//

#ifndef XPLAY2_XVIDEOTHREAD_HPP
#define XPLAY2_XVIDEOTHREAD_HPP

#include "XAVQThreadAbstract.hpp"
#include "IVideoCall.hpp"

class XVideoThread : public XAVQThreadAbstract {
Q_OBJECT
    void run() override;
    void Open(const XAVCodecParameters_sptr &) noexcept(false) override;
public:
    explicit XVideoThread(std::exception_ptr * = nullptr);
    void Open(const XAVCodecParameters_sptr &,IVideoCall *) noexcept(false);
    using XAVQThreadAbstract::Push;
    using XAVQThreadAbstract::SetException_ptr;

protected:
    //std::atomic<IVideoCall*> m_call{};
    IVideoCall* m_call{};
public:
    ~XVideoThread() override;
};


#endif //XPLAY2_XVIDEOTHREAD_HPP
