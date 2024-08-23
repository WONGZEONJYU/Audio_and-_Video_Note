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
#include "XSonic.hpp"

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

    m_Channels = p->Ch_layout()->nb_channels;
    m_Sample_Format = QAudioFormat::Int16;
    m_Sample_Rate = p->Sample_rate();

    try {
        XDecodeThread::Open(p);
        {
            QMutexLocker locker(&m_a_mux);

            if (!m_resample) {
                CHECK_EXC(m_resample.reset(new XResample()),locker.unlock());
            }

            if (!m_xSonic) {
                CHECK_EXC(m_xSonic.reset(new XSonic()),locker.unlock());
            }

            if(!m_xSonic->Open(m_Sample_Rate,m_Channels)){
                throw std::runtime_error(GET_STR(m_xSonic->Open(m_Sample_Rate,m_Channels) failed));
            }
        }

        m_resample->Open(p);
        m_audio_play.load()->set_Audio_parameter(m_Sample_Rate,m_Channels,QAudioFormat::Int16);
    } catch (...) {
        DeConstruct();
        throw;
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
                const auto no_playMs{m_audio_play.load()->NoPlayMs()};
                m_pts = pts - no_playMs;

                int sonic_size{};
                {
                    QMutexLocker locker(&m_a_mux);
                    int re_size{},out_samples{};
                    CHECK_EXC(re_size = m_resample->Resample(af,resample_datum,out_samples),
                              locker.unlock());

                    if (m_speed != 1.0f){
                        if (out_samples > 0) {
                            m_xSonic->sonicSetSpeed(m_speed);
                            m_xSonic->sonicWriteShortToStream(reinterpret_cast<int16_t *>(resample_datum.data()),out_samples);
                            const auto size_{static_cast<int>(static_cast<float >(re_size) / m_speed) * sizeof(int16_t)};
                            if (speed_datum.capacity() <= size_) {
                                speed_datum.clear();
                                speed_datum.resize(size_ + (size_ >> 1));
                            }

                            sonic_size = m_xSonic->sonicReadShortFromStream(reinterpret_cast<int16_t*>(speed_datum.data()),
                                                                            static_cast<int>(static_cast<float>(out_samples) / m_speed));

                            if (sonic_size > 0){
                                sonic_size = sonic_size * m_Channels * m_Sample_Format;
                            }
                        }
                    } else {
                        //qDebug() << "re_size: " << re_size;
                        sonic_size = re_size;
                        speed_datum = std::move(resample_datum);
                    }
                }

            while (!m_is_Exit && sonic_size > 0){
                //qDebug() << "sonic_size: " << sonic_size;
                const auto free_size{m_audio_play.load()->FreeSize()};
                //qDebug() << "free_size: " << free_size;
                if (m_is_Pause || free_size < sonic_size) {
                    msleep(1);
                    continue;
                }

                CHECK_EXC(m_audio_play.load()->Write(speed_datum.data(),sonic_size));
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
