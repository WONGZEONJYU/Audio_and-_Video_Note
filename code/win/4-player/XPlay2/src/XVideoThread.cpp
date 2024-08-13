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

void XVideoThread::entry() {

    try {
        while (!m_is_Exit) {
            int64_t pts{};
            while (!m_is_Exit) {
                XAVFrame_sptr vf;
                CHECK_EXC(vf = Receive_Frame(pts));//有异常
                if (!vf){
                    break;
                }
                m_call.load()->Repaint(vf);
            }

            if (m_sync_pts > 0 && m_sync_pts < pts){
                msleep(1);
                continue;
            }

            bool b;
            CHECK_EXC(b = Send_Packet(Pop()));
            if (b){
                PopFront();
            }else{
                msleep(1);
                continue;
            }
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

    Clear();
    m_call = call;
    m_call.load()->Init(p->Width(),p->Height());
    XDecodeThread::Open(p);
}
