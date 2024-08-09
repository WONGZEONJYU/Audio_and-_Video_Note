//
// Created by Administrator on 2024/8/9.
//

#ifndef XPLAY2_XVIDEOTHREAD_HPP
#define XPLAY2_XVIDEOTHREAD_HPP

#include "XAVQThread_Abstract.hpp"
#include "IVideoCall.hpp"
#include "XHelper.hpp"

class XVideoThread : public XAVQThread_Abstract {

    void run() override;
    void Open(const XAVCodecParameters_sptr &) noexcept(false) override;
public:
    explicit XVideoThread(IVideoCall* = nullptr,std::exception_ptr * = nullptr);
    void Open(const XAVCodecParameters_sptr &,IVideoCall *) noexcept(false);
    using XAVQThread_Abstract::SetException_ptr;

protected:
    std::atomic<IVideoCall*> m_call{};
private:

public:
    ~XVideoThread() override;
};


#endif //XPLAY2_XVIDEOTHREAD_HPP
