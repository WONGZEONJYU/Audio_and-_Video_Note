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

XAudioThread::XAudioThread(std::exception_ptr *e) : XAVQThreadAbstract(e),
                                                    m_audio_play{QXAudioPlay::handle()} {

}

void XAudioThread::Open(const XAVCodecParameters_sptr &p) noexcept(false) {

    if (!p){
        PRINT_ERR_TIPS(GET_STR(XAVCodecParameters_sptr is empty));
        return;
    }

    QMutexLocker lock(&m_re_mux);

    try {
        if (!m_decode){
            m_decode.reset(new XDecode);
        }

        if (!m_resample){
            m_resample.reset(new XResample);
        }

//        if (!m_audio_play){
//            m_audio_play = QXAudioPlay::handle();
//        }

        m_resample->Open(p);
        m_audio_play->set_Audio_parameter(p->Sample_rate(),p->Ch_layout()->nb_channels,QAudioFormat::Int16);
        m_decode->Open(p);

    } catch (...) {
        DeConstruct();
        lock.unlock();
        throw ;
    }
}

void XAudioThread::DeConstruct() noexcept(true){

    Exit_Thread();

    if (m_decode){
        m_decode.reset();
    }

    if (m_resample){
        m_resample.reset();
    }
}

XAudioThread::~XAudioThread(){
    DeConstruct();
}

void XAudioThread::run() noexcept(false) {

    QMutexLocker lock(&m_re_mux);

    try {
        m_audio_play->Open();

        while (!m_is_Exit) {
            qDebug() << currentThreadId();

            if (!m_decode || !m_resample){
                PRINT_ERR_TIPS(GET_STR(Please open first));
                lock.unlock();
                msleep(1);
                lock.relock();
                continue;
            }

            while (!m_is_Exit) {

                const auto af{m_decode->Receive()};//可能抛异常
                if (!af){
                    break;
                }

                const auto re_size{m_resample->Resample(af, m_resample_datum)};

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
                    m_audio_play->Write(m_resample_datum.data(),re_size);
                    break;
                }
            }

            if (m_Packets.isEmpty()){
                lock.unlock();
                msleep(1);
                lock.relock();
                continue;
            }

            const auto b {m_decode->Send(m_Packets.first())};
            if (b){
                m_Packets.removeFirst();
            }
        }
        m_audio_play->Close();
    } catch (...) {
        m_audio_play->Close();
        lock.unlock();
        if (m_exceptionPtr){
            *m_exceptionPtr = current_exception();
        }
    }
}
