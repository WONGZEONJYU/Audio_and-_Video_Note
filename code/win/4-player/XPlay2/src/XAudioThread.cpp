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
    m_Out_Sample_Format = QAudioFormat::Int16;
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
                locker.unlock();
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

int XAudioThread::Resample(XAVFrame_sptr &&af,std::vector<uint8_t> &d,int &out_samples) noexcept(false){

    QMutexLocker locker(&m_a_mux);
    int re_size;
    CHECK_EXC(re_size = m_resample->Resample(af,d,out_samples),
              locker.unlock());
    return re_size;
}

int XAudioThread::Speed_Change(std::vector<uint8_t> &in,
                               const int &in_samples,
                               const int &in_re_size,
                               std::vector<uint8_t> &out){
    int out_size{};

    if (m_speed != 1.0f) {

        if (in_samples > 0 && in_re_size > 0) {

            {
                QMutexLocker locker(&m_a_mux);

                m_xSonic->sonicSetSpeed(m_speed);

                m_xSonic->sonicWriteShortToStream(reinterpret_cast<const int16_t *>(in.data()), in_samples);

                const auto size_{static_cast<uint32_t>(static_cast<float >(in_re_size) / m_speed) * sizeof(int16_t)};
                if (out.capacity() <= size_) {
                    out.clear();
                    out.resize(size_ + (size_ >> 1));
                }

                const auto out_samples{static_cast<int>(static_cast<float>(in_samples) / m_speed)};
                out_size = m_xSonic->sonicReadShortFromStream(reinterpret_cast<int16_t *>(out.data()),out_samples);
            }

            if (out_size > 0) {
                out_size *= m_Channels * m_Out_Sample_Format;
            }
        }
    }else{
        out_size = in_re_size;
        out = std::move(in);
    }

    return out_size;
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

                int out_samples{};
                const auto re_size{Resample(std::move(af),resample_datum,out_samples)};

                const auto sonic_size{Speed_Change(resample_datum,out_samples,re_size,speed_datum)};

                while (!m_is_Exit && sonic_size > 0){
                    const auto free_size{m_audio_play.load()->FreeSize()};
                    if (m_is_Pause || free_size < sonic_size) {
                        msleep(1);
                        continue;
                    }

                    CHECK_EXC(m_audio_play.load()->Write(speed_datum.data(),sonic_size));
                    break;
                }
            }

            if (!Send_Packet()) {
                msleep(1);
            }
        }

    } catch (...) {
        //qDebug() << GET_STR(XAudioThread::) <<__func__ << "catch";
        if (m_exceptionPtr){
            *m_exceptionPtr = current_exception();
        }
    }

    m_audio_play.load()->Close();
}

void XAudioThread::Close() noexcept(true) {
    XDecodeThread::Close();
    QMutexLocker locker(&m_a_mux);
    m_resample.reset();
    m_xSonic.reset();
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
