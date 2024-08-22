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
            m_xSonic = XSonic(p->Sample_rate(),p->Ch_layout()->nb_channels);
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

    std::vector<uint8_t> resample_datum,speed_datum;
    try {
        m_audio_play.load()->Open();

        while (!m_is_Exit) {

            if (m_is_Pause){
                msleep(5);
                continue;
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

                /*获取时间差,用于同步*/
                m_pts = pts - m_audio_play.load()->NoPlayMs();

                int re_size{},sonic_size{},out_samples{};
                {
                    QMutexLocker locker(&m_a_mux);
                    CHECK_EXC(re_size = m_resample->Resample(af, resample_datum,out_samples),
                              locker.unlock());
#if 1
                    if (out_samples > 0) {
                        static constexpr auto speed{1.3f};
                        m_xSonic.sonicSetSpeed(speed);
                        m_xSonic.sonicWriteShortToStream(reinterpret_cast<int16_t *>(resample_datum.data()),out_samples);
                        if (speed_datum.capacity() <= static_cast<int>(re_size / speed) * sizeof(int16_t)){
                            speed_datum.resize(static_cast<int>(re_size / speed) * sizeof(int16_t) + 2);
                        }
                        //qDebug() << speed_datum.capacity();
                        sonic_size = m_xSonic.sonicReadShortFromStream(reinterpret_cast<int16_t*>(speed_datum.data()),
                                                                       static_cast<int>(out_samples / speed));
                        //qDebug() << sonic_size;
                        if (sonic_size > 0){
                            sonic_size = sonic_size * 4;
                        }
                    }
#endif
                }
#if 0
                while (!m_is_Exit && re_size > 0) {

                    if (m_is_Pause || m_audio_play.load()->FreeSize() < re_size) {
                        msleep(1);
                        continue;
                    }

                    CHECK_EXC(m_audio_play.load()->Write(resample_datum.data(),re_size));
                    break;
                }
            }
#else
            while (!m_is_Exit && sonic_size > 0){
                qDebug() << "sonic_size: " << sonic_size;
                qDebug() << "buffer_size: " << m_audio_play.load()->BufferSize();
                const auto free_size {m_audio_play.load()->FreeSize()};
                if (m_is_Pause || free_size < sonic_size) {
                    msleep(1);
                    continue;
                }

                CHECK_EXC(m_audio_play.load()->Write(speed_datum.data(),sonic_size));
                break;
            }
        }

#endif

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
        qDebug() << GET_STR(XAudioThread::) <<__func__ << "catch";
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
