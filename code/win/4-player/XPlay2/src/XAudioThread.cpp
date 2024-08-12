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

XAudioThread::XAudioThread(std::exception_ptr *e) : XDecodeThread(e),
                                                    m_audio_play{QXAudioPlay::handle()} {
}

XAudioThread::~XAudioThread() {
    DeConstruct();
}

void XAudioThread::Open(const XAVCodecParameters_sptr &p) noexcept(false) {

    if (!p){
        PRINT_ERR_TIPS(GET_STR(XAVCodecParameters_sptr is empty));
        return;
    }

    m_pts = m_sync_pts = 0;

    QMutexLocker lock(&m_mux);

    try {
        Create_Decode();

        if (!m_resample){
            m_resample.reset(new XResample);
        }

        m_resample->Open(p);
        m_audio_play->set_Audio_parameter(p->Sample_rate(),p->Ch_layout()->nb_channels,QAudioFormat::Int16);
        m_decode->Open(p);

        m_wc.wakeAll();

    } catch (...) {
        DeConstruct();
        lock.unlock();
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
            QMutexLocker lock(&m_mux);
            m_audio_play->Open();
        }

        while (!m_is_Exit) {

            QMutexLocker lock(&m_mux);

            if (!m_decode || !m_resample) {
                PRINT_ERR_TIPS(GET_STR(Please open first));
                m_wc.wait(&m_mux,1);
                continue;
            }

            while (!m_is_Exit) {
                XAVFrame_sptr af;
                CHECK_EXC(af = m_decode->Receive(),lock.unlock()); //可能抛异常
                if (!af){
                    break;
                }

                /*
                 *获取时间差
                 */
                m_pts = m_decode->Pts() - m_audio_play->NoPlayMs();

                int re_size{};
                CHECK_EXC(re_size = m_resample->Resample(af, resample_datum),lock.unlock());

                while (!m_is_Exit) {

                    if (re_size <= 0){ //不能放在外层判断,此处需循环判断音频播放是否空闲
                        break;
                    }

                    if (m_audio_play->FreeSize() < re_size) {
                        lock.unlock();
                        msleep(1);
                        lock.relock();
                        continue;
                    }
                    CHECK_EXC(m_audio_play->Write(resample_datum.data(),re_size),lock.unlock());
                    break;
                }
            }

//            if (m_Packets.isEmpty()) {
//                m_wc.wait(&m_mux,1);
//                continue;
//            }

            bool b;
            CHECK_EXC(b = m_decode->Send(Pop()),lock.unlock());
            if (b){
                PopFront();
                m_wc.wakeAll();
            }else{
                m_wc.wait(&m_mux,1);
            }
        }

        {
            QMutexLocker lock(&m_mux);
            m_audio_play->Close();
        }
    } catch (...) {
        {
            QMutexLocker lock(&m_mux);
            m_audio_play->Close();
        }
        if (m_exceptionPtr){
            *m_exceptionPtr = current_exception();
        }
    }
}
