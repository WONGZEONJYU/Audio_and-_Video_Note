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

            if (m_is_Pause) {
                msleep(5);
                continue;
            }

            while (!m_is_Exit) {
                XAVFrame_sptr vf;
                int64_t pts{};
                CHECK_EXC(vf = Receive_Frame(pts));//有异常
                if (!vf) {
                    break;
                }

                m_pts = pts;

                while (!m_is_Exit) {
                    const auto sync_pts {m_sync_pts.load()};

                    if (m_has_audio){
                        if ( m_is_Pause || (0 < sync_pts && sync_pts < pts) ) {
                            qDebug() << "sync_pts: " << sync_pts;
                            msleep(1);
                            continue;
                        }
                        m_call.load()->Repaint(vf);
                    }else{
                        const auto delay_ms{static_cast<unsigned long>(40.0 / m_speed)};
                        m_call.load()->Repaint(vf);
                        msleep(delay_ms);
                    }
                    break;
                }
            }

            if (!Send_Packet()) {
                msleep(1);
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

    if (m_call != call){
        m_call = call;
    }

    m_call.load()->Init(p->Width(),p->Height());

    XDecodeThread::Open(p);
}

bool XVideoThread::RepaintPts(const XAVPacket_sptr &pkt,
                              const int64_t &seek_pts) noexcept(false) {

    auto b {!Send_Packet(pkt)}; //如果解码失败当已经显示成功,让上一级函数结束,
    // 因为在同一个位置读取的是同一包,即使再次读取还是会解码失败
    if (b) {return b;}

    do {
        int64_t pts{};
        const auto frame{Receive_Frame(pts)};
        if (!frame) {
            b = false;
            break;
        }

        if (pts >= seek_pts) {
            m_call.load()->Repaint(frame);
            b = true;
            break;
        }
    } while (!m_is_Exit);
    return b;
}

void XVideoThread::SetPause(const bool &b) noexcept(true){
    XDecodeThread::SetPause(b);
}

void XVideoThread::Close() noexcept(true) {
    XDecodeThread::Close();
    //m_last_pts = 0;
}
