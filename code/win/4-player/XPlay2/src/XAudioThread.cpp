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
    m_a_cv.wakeAll();
    Exit_Thread();
}

void XAudioThread::entry() noexcept(false) {

    std::vector<uint8_t> resample_datum;
    try {

        m_audio_play.load()->Open();

        while (!m_is_Exit) {

            if (m_is_Pause){
                m_a_cv.wait(&m_a_mux);
                m_a_mux.unlock();
            }

            if (m_audio_play.load()->Is_Transform()){ //中途改变媒体文件需要重新打开
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

                int re_size{};
                {
                    QMutexLocker locker(&m_a_mux);
                    CHECK_EXC(re_size = m_resample->Resample(af, resample_datum),
                              locker.unlock());
                }

                while (!m_is_Exit && re_size > 0) {

                    if (m_is_Pause || m_audio_play.load()->FreeSize() < re_size) {
                        msleep(1);
                        continue;
                    }

                    CHECK_EXC(m_audio_play.load()->Write(resample_datum.data(),re_size));
                    break;
                }
            }
#if 0
            if (Empty()){
                msleep(1);
                continue;
            }

            bool b;
            CHECK_EXC(b = Send_Packet(Pop()));
            if (b){
                PopFront();
            }
#else
            if (!Send_Packet()){
                msleep(1);
            }
#endif
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
    QMutexLocker locker(&m_a_mux);
    m_resample.reset();
}

void XAudioThread::SetPause(const bool &b) noexcept(true){
    XDecodeThread::SetPause(b);
    if (m_audio_play){
        m_audio_play.load()->SetPause(b);
    }
    if (!b){
        m_a_cv.wakeAll();
    }
}

void XAudioThread::Clear() noexcept(true) {
    XDecodeThread::Clear();
    m_audio_play.load()->Clear();
}

void XAudioThread::SetVolume(const double &n) noexcept(true){
    m_audio_play.load()->SetVolume(n);
}

double XAudioThread::Volume() const noexcept(true){
    return m_audio_play.load()->Volume();
}
