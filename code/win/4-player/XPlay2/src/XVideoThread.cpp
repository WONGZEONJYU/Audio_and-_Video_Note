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

    m_pts = 0;
    m_sync_pts = 0;

    QMutexLocker locker(&m_mux);

    if (!m_decode){
        CHECK_EXC(m_decode.reset(new XDecode()),locker.unlock());
        m_decode->Open(p);
        m_wc.wakeAll();
    }
}

void XVideoThread::run() {

    try {
        while (!m_is_Exit) {
            QMutexLocker locker(&m_mux);

            if (!m_decode || !m_call){
                PRINT_ERR_TIPS(GET_STR(Please open first));
                m_wc.wait(&m_mux,1);
                continue;
            }

            while (!m_is_Exit){
                XAVFrame_sptr vf;
                CHECK_EXC(vf = m_decode->Receive(),locker.unlock());//有异常
                if (!vf){
                    break;
                }
                m_call.load()->Repaint(vf);
            }

            if (m_Packets.isEmpty()){
                m_wc.wait(&m_mux,1);
                continue;
            }

            if (m_sync_pts < m_decode->Pts()){
                locker.unlock();
                msleep(1);
                continue;
            }

            bool b;
            CHECK_EXC(b = m_decode->Send(m_Packets.first()),locker.unlock());
            if (b){
                m_Packets.removeFirst();
                m_wc.wakeAll();
            }
        }
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

    {
        QMutexLocker locker(&m_mux);
        m_call = call;
        m_call.load()->Init(p->Width(),p->Height());
    }

    Open(p);
}
