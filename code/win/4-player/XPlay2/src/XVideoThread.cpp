//
// Created by Administrator on 2024/8/9.
//

#include "XVideoThread.hpp"
#include "XDecode.hpp"
#include "IVideoCall.hpp"
#include "XAVCodecParameters.hpp"

XVideoThread::XVideoThread(std::exception_ptr *p): XAVQThreadAbstract(p){

}

XVideoThread::~XVideoThread() {
    Exit_Thread();
}

void XVideoThread::Open(const XAVCodecParameters_sptr &p) noexcept(false) {

    QMutexLocker locker(&m_re_mux);
    if (!m_decode){
        CHECK_EXC(m_decode.reset(new XDecode()),locker.unlock());
    }
    m_decode->Open(p);
}

void XVideoThread::run() {

    QMutexLocker locker(&m_re_mux);

    try {
        while (!m_is_Exit){
            //qDebug() << currentThreadId() ;
            if (!m_decode || !m_call){
                PRINT_ERR_TIPS(GET_STR(Please open first));
                locker.unlock();
                msleep(1);
                continue;
            }

            while (!m_is_Exit){

                const auto vf{m_decode->Receive()};//有异常
                if (!vf){
                    break;
                }
                //m_call.load()->Repaint(vf);
                m_call->Repaint(vf);
                msleep(40);
            }

            if (m_Packets.isEmpty()){
                locker.unlock();
                msleep(1);
                locker.relock();
                continue;
            }

            const auto b{ m_decode->Send(m_Packets.first()) };
            if (b){
                m_Packets.removeFirst();
            }
        }
    } catch (...) {
        locker.unlock();
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
    //m_call.load()->Init(p->Width(),p->Height());
    m_call->Init(p->Width(),p->Height());
    Open(p);
}
