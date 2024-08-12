//
// Created by Administrator on 2024/8/9.
//

#include "XVideoThread.hpp"
#include "XDecode.hpp"
#include "IVideoCall.hpp"
#include "XAVCodecParameters.hpp"
#include <QDebug>

XVideoThread::XVideoThread(std::exception_ptr *p): XDecodeThread(p){
}

XVideoThread::~XVideoThread() {
    Exit_Thread();
}

void XVideoThread::Open(const XAVCodecParameters_sptr &p) noexcept(false) {

    {
        QMutexLocker locker(&m_v_mux);
        m_pts = m_sync_pts = 0;
    }
    Create_Decode();

    m_decode->Open(p);
}

void XVideoThread::entry() {

    try {
        while (!m_is_Exit) {
            QMutexLocker locker(&m_v_mux);
            //qDebug() << GET_STR(XVideoThread::) << __func__ ;

//            while (!m_is_Exit){
//                XAVFrame_sptr vf;
//                CHECK_EXC(vf = m_decode->Receive(),locker.unlock());//有异常
//                if (!vf){
//                    break;
//                }
//                m_call.load()->Repaint(vf);
//            }
//
//            if (m_sync_pts > 0 && m_sync_pts < m_decode->Pts()){
//                locker.unlock();
//                msleep(1);
//                continue;
//            }

            Pop();
            PopFront();;

//            bool b;
//            CHECK_EXC(b = m_decode->Send(Pop()),locker.unlock());
//            if (b){
//                PopFront();
//            }else{
//                locker.unlock();
//                msleep(1);
//                continue;
//            }
        }
    } catch (...) {
        qDebug() << __func__ << "catch";
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
        QMutexLocker locker(&m_v_mux);
        m_call = call;
        m_call.load()->Init(p->Width(),p->Height());
    }

    Open(p);
}
