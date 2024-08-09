//
// Created by Administrator on 2024/8/9.
//

#include "XVideoThread.hpp"
#include "XDecode.hpp"
#include "XVideoWidget.hpp"
#include "IVideoCall.hpp"
#include "XAVCodecParameters.hpp"

XVideoThread::XVideoThread(IVideoCall *call,std::exception_ptr *p):
XAVQThread_Abstract(p),m_call{call}{

}

XVideoThread::~XVideoThread() {
    Exit_Thread();
}

void XVideoThread::Open(const XAVCodecParameters_sptr &p) noexcept(false) {

    QMutexLocker locker(&m_re_mux);
    try{
        if (!m_decode){
            CHECK_EXC(m_decode.reset(new XDecode()));
        }
        m_decode->Open(p);

    } catch (...) {
        locker.unlock();
        throw;
    }
}

void XVideoThread::run() {

    QMutexLocker locker(&m_re_mux);
    try {

    } catch (...) {
        if (m_exceptionPtr){
            *m_exceptionPtr = std::current_exception();
        }
    }
}

void XVideoThread::Open(const XAVCodecParameters_sptr &p, IVideoCall *call) noexcept(false) {

    if (!p) {
        PRINT_ERR_TIPS(GET_STR(XAVCodecParameters_sptr is empty));
        return;
    }

    if (!call) {
        PRINT_ERR_TIPS(GET_STR(call is empty));
        return;
    }

    QMutexLocker locker(&m_re_mux);
    m_call = call;
    m_call.load()->Init(p->Width(),p->Height());
    Open(p);
}
