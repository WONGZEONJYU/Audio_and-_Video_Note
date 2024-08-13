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
XDecodeThread(e),m_audio_play(QXAudioPlay::handle()) {


}

XAudioThread::~XAudioThread() {
    DeConstruct();
}

void XAudioThread::Open(const XAVCodecParameters_sptr &p) noexcept(false) {

    if (!p) {
        PRINT_ERR_TIPS(GET_STR(XAVCodecParameters_sptr is empty));
        return;
    }

    try {
        XDecodeThread::Open(p);

        {
            QMutexLocker locker(&m_a_mux);
            if (!m_resample) {
                CHECK_EXC(m_resample.reset(new XResample()),locker.unlock());
            }
        }

        m_resample->Open(p);
        m_audio_play.load()->set_Audio_parameter(p->Sample_rate(),
                                          p->Ch_layout()->nb_channels,
                                          QAudioFormat::Int16);

    } catch (...) {
        DeConstruct();
        throw ;
    }
}

void XAudioThread::DeConstruct() noexcept(true){
    Exit_Thread();
}

void XAudioThread::entry() noexcept(false) {

    try {

        m_audio_play.load()->Open();

        std::vector<uint8_t> resample_datum;

        while (!m_is_Exit) {

            if (m_audio_play.load()->Is_Transform()){
                m_audio_play.load()->Open();
            }

            while (!m_is_Exit) {

                XAVFrame_sptr af;
                int64_t pts{};
                CHECK_EXC(af = Receive_Frame(pts)); //可能抛异常
                if (!af) {
                    break;
                }

                /*
                 *获取时间差,用于同步
                 */
                m_pts = pts - m_audio_play.load()->NoPlayMs();

                QMutexLocker locker(&m_a_mux);
                int re_size{};
                CHECK_EXC(re_size = m_resample->Resample(af, resample_datum),
                          locker.unlock());

                while (!m_is_Exit) {

                    if (re_size <= 0){ //不能放在外层判断,此处需循环判断音频播放是否空闲
                        break;
                    }

                    if (m_audio_play.load()->FreeSize() < re_size) {
                        msleep(1);
                        continue;
                    }

                    CHECK_EXC(m_audio_play.load()->Write(resample_datum.data(),re_size));
                    break;
                }
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

        m_audio_play.load()->Close();

    } catch (...) {
        m_audio_play.load()->Close();
        qDebug() << __func__ << "catch";
        if (m_exceptionPtr){
            *m_exceptionPtr = current_exception();
        }
    }
}

void XAudioThread::Close() noexcept(true) {
    XDecodeThread::Close();
    m_resample->Close();
    QMutexLocker locker(&m_a_mux);
    m_resample.reset();
}
