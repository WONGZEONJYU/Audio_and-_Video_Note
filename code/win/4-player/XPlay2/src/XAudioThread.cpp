//
// Created by wong on 2024/8/8.
//

#include "XAudioThread.hpp"
#include "XDecode.hpp"
#include "XResample.hpp"
#include "QXAudioPlay.hpp"
#include "XAVCodecParameters.hpp"

using namespace std;

XAudioThread::XAudioThread(std::exception_ptr *e) :QThread(), m_exceptionPtr(e){

}

void XAudioThread::Open(const XAVCodecParameters_sptr &p) noexcept(false) {

    if (!p){
        PRINT_ERR_TIPS(GET_STR(XAVCodecParameters_sptr is empty));
        return;
    }
    //Close();
    QMutexLocker lock(&m_re_mux);
    try {
        if (!m_decode){
            CHECK_EXC(m_decode.reset(new XDecode));
        }

        if (!m_resample){
            CHECK_EXC(m_resample.reset(new XResample));
        }

        if (!m_audio_play){
            m_audio_play = QXAudioPlay::handle();
        }

        m_resample->Open(p);
        m_audio_play->set_Audio_parameter(p->Sample_rate(),p->Ch_layout()->nb_channels,QAudioFormat::Int16);
        m_decode->Open(p);

    } catch (...) {
        DeConstruct();
        lock.unlock();
        //Close();
        throw ;
    }
}

//void XAudioThread::Close() noexcept(true) {
//    QMutexLocker lock(&m_re_mux);
//    DeConstruct();
//}

void XAudioThread::DeConstruct() noexcept(true){

    m_is_Exit = true;
    quit();
    wait();

    m_audio_play = nullptr;

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

    try {

        {
            QMutexLocker lock(&m_re_mux);
            m_audio_play->Open();
        }

        while (!m_is_Exit){
            QMutexLocker lock(&m_re_mux);

            if (m_Packets.isEmpty()){
                lock.unlock();
                msleep(1);
            }
            lock.relock();
            CHECK_EXC(m_decode->Send(m_Packets.first()),lock.unlock());

            while (true){
                XAVFrame_sptr af;
                CHECK_EXC(af = m_decode->Receive(),lock.unlock());
                if (!af){
                    break;
                }
            }
        }

        {
            QMutexLocker lock(&m_re_mux);
            m_audio_play->Close();
        }
    } catch (...) {
        m_audio_play->Close();
        if (m_exceptionPtr){
            *m_exceptionPtr = current_exception();
        }
    }
}

void XAudioThread::Push(XAVPacket_sptr &&pkt) {

    if (!pkt){
        PRINT_ERR_TIPS(GET_STR(XAVPacket_sptr is empty));
        return;
    }

    while (!m_is_Exit) {
        {
            QMutexLocker lock(&m_re_mux);
            if (m_Packets.size() < Max_List){
                m_Packets.push_back(pkt);
                break;
            }
        }
        msleep(1);
    }
}
