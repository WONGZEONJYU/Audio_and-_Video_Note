//
// Created by wong on 2024/8/8.
//
#include <QAudioFormat>
#include <QDebug>
#include "XAudioThread.hpp"
#include "XDecode.hpp"
#include "XResample.hpp"
#include "QXAudioPlay.hpp"
#include "XAVCodecParameters.hpp"

using namespace std;

XAudioThread::XAudioThread(std::exception_ptr *e) :
XDecodeThread(e),m_audio_play{QXAudioPlay::handle()}{

}

XAudioThread::~XAudioThread() {
    DeConstruct();
}

void XAudioThread::Open(const XAVCodecParameters_sptr &p) noexcept(false) {

    if (!p){
        PRINT_ERR_TIPS(GET_STR(XAVCodecParameters_sptr is empty));
        return;
    }

    QMutexLocker locker(&m_a_mux);
    m_pts = m_sync_pts = 0;

    try {
        Create_Decode();
        if (!m_resample){
            m_resample.reset(new XResample);
        }

        m_resample->Open(p);
        m_audio_play->set_Audio_parameter(p->Sample_rate(),
                                          p->Ch_layout()->nb_channels,
                                          QAudioFormat::Int16);
        m_decode->Open(p);

    } catch (...) {
        DeConstruct();
        locker.unlock();
        throw ;
    }
}

void XAudioThread::DeConstruct() noexcept(true){
    Exit_Thread();
}

void XAudioThread::entry() noexcept(false) {
    std::vector<uint8_t> resample_datum;
    try {

        {
            QMutexLocker lock(&m_a_mux);
            m_audio_play->Open();
        }

        while (!m_is_Exit) {

            QMutexLocker locker(&m_a_mux);
            //qDebug() << GET_STR(XAVideoThread::) << __func__ ;
            while (!m_is_Exit) {

                XAVFrame_sptr af;
                CHECK_EXC(af = m_decode->Receive(),locker.unlock()); //可能抛异常
                if (!af){
                    break;
                }

                /*
                 *获取时间差
                 */
                m_pts = m_decode->Pts() - m_audio_play->NoPlayMs();
                qDebug() << "m_decode->Pts() - m_audio_play->NoPlayMs() = " << m_pts;
                int re_size{};
                CHECK_EXC(re_size = m_resample->Resample(af, resample_datum),
                          locker.unlock());

                while (!m_is_Exit) {

                    if (re_size <= 0){ //不能放在外层判断,此处需循环判断音频播放是否空闲
                        break;
                    }

                    if (m_audio_play->FreeSize() < re_size) {
                        msleep(1);
                        continue;
                    }

                    CHECK_EXC(m_audio_play->Write(resample_datum.data(),re_size));
                    break;
                }
            }


            auto pkt = Pop();

            bool b;
            CHECK_EXC(b = m_decode->Send(pkt),locker.unlock());
            if (b){
                PopFront();
            }else{
                locker.unlock();
                msleep(1);
                continue;
            }
        }

        {
            QMutexLocker lock(&m_a_mux);
            m_audio_play->Close();
        }
    } catch (...) {
        {
            QMutexLocker lock(&m_a_mux);
            m_audio_play->Close();
        }
        qDebug() << __func__ << "catch";
        if (m_exceptionPtr){
            *m_exceptionPtr = current_exception();
        }
    }
}
