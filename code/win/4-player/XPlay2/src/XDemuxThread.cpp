//
// Created by wong on 2024/8/10.
//
#include <QDebug>
#include "XDemuxThread.hpp"
#include "XAudioThread.hpp"
#include "XVideoThread.hpp"
#include "XAVCodecParameters.hpp"
#include "XDemux.hpp"
#include "IVideoCall.hpp"
#include "XAVPacket.hpp"

XDemuxThread::XDemuxThread(std::exception_ptr *e) :
    QThread(), m_ex_ptr(e) {
}

XDemuxThread::~XDemuxThread() {
    DeConstruct();
}

void XDemuxThread::DeConstruct() noexcept(true) {
    m_is_Exit = true;
    m_cv.wakeAll();
    m_at->Close();
    m_vt->Close();
    quit();
    wait();
}

void XDemuxThread::Open(const QString &url,IVideoCall *call) noexcept(false) {

    if (url.isEmpty()){
        PRINT_ERR_TIPS(GET_STR(url is empty));
        return;
    }

    if (!call){
        PRINT_ERR_TIPS(GET_STR(IVideoCall is empty));
        return;
    }

    QMutexLocker locker(&m_mux);

    try {

        if (!m_demux){
            m_demux.reset(new XDemux());
        }

        if (!m_at){
            m_at.reset(new XAudioThread(m_ex_ptr.load()));
        }

        if (!m_vt){
            m_vt.reset(new XVideoThread(m_ex_ptr.load()));
        }

        m_demux->Open(url.toStdString());
        m_ac = m_demux->Copy_Present_AudioCodecParam();
        m_vc = m_demux->Copy_Present_VideoCodecParam();

        m_at->Open(m_ac);
        m_vt->Open(m_vc,call);
        m_total_Ms = m_demux->totalMS();
        m_cv.wakeAll();

    } catch (...) {
        locker.unlock();
        throw;
    }
}

void XDemuxThread::run() {

    try {
        bool end{};
        while (!m_is_Exit){

            if (m_isPause){
                msleep(5);
                continue;
            }

            const auto a_index {m_demux->Present_Audio_Index()},
                    v_index {m_demux->Present_Video_Index()};
            /**
             * 音频获取到到pts给视频进行同步
             */
            m_vt->Set_Sync_Pts(m_at->Pts());

            /**
             * 用于进度条
             */
            m_pts = m_at->Pts();

            XAVPacket_sptr pkt;
            QMutexLocker locker(&m_mux);
            CHECK_EXC(pkt = m_demux->Read()); //可能有异常
            if (!pkt && !end){
                end = true;
                if (a_index >= 0){
                    m_at->Push({});
                }
                if (v_index >= 0){
                    m_vt->Push({});
                }
                m_cv.wait(&m_mux,1);
            }else if (!pkt) {
                m_cv.wait(&m_mux,1);
            }else{
                end = false;
                const auto pkt_index {pkt->stream_index};
                if (a_index == pkt_index){
                    m_at->Push(std::move(pkt));
                }else if (v_index == pkt_index){
                    m_vt->Push(std::move(pkt));
                } else{
                    pkt.reset();
                }
                locker.unlock();
                msleep(1);
            }
        }

    } catch (...) {
        if (m_ex_ptr){
            *m_ex_ptr = std::current_exception();
        }
    }
}

void XDemuxThread::Start() noexcept(false) {

    QMutexLocker locker(&m_mux);

    if (!m_demux){
        m_demux.reset(new XDemux());
    }

    if (!m_at){
        m_at.reset(new XAudioThread());
    }

    if (!m_vt) {
        m_vt.reset(new XVideoThread());
    }

    start();

    if (m_at){
        m_at->start();
    }

    if (m_vt){
        m_vt->start();
    }
}

void XDemuxThread::Close() noexcept(false) {
    DeConstruct();
    m_demux.reset();
    m_at.reset();
    m_vt.reset();
}

void XDemuxThread::SetPause(const bool &b){
    m_isPause = b;
    QMutexLocker locker(&m_mux);
    if (m_at){
        m_at->SetPause(b);
    }
    if (m_vt){
        m_vt->SetPause(b);
    }
}

void XDemuxThread::Seek(const double &pos) noexcept(true) {

    Clear();
    const auto status {m_isPause.load()};
    SetPause(true); //无论当前什么状态,都先暂停再seek

    QMutexLocker locker(&m_mux);
    if (m_demux){
        m_demux->Seek(pos);
    }

    const auto t_pos {pos * static_cast<decltype(pos)>(m_total_Ms.load())};
    const auto seek_pts {static_cast<int64_t>(t_pos)};

    while (!m_is_Exit) {
        XAVPacket_sptr pkt;
        CHECK_EXC(pkt = m_demux->ReadVideo(),locker.unlock());
        if (!pkt) {
            break;
        }
        if (m_vt->RepaintPts(pkt,seek_pts)) {
            m_pts = seek_pts; //更新当前pts
            break;
        }
    }
    locker.unlock();
    SetPause(status);
}

void XDemuxThread::Clear() noexcept(true) {
    QMutexLocker locker(&m_mux);
    if (m_demux){
        m_demux->Clear();
    }
    if (m_at){
        m_at->Clear();
    }
    if (m_vt){
        m_vt->Clear();
    }
}

void XDemuxThread::SetVolume(const double &n) noexcept(true){
    QMutexLocker locker(&m_mux);
    if (m_at){
        m_at->SetVolume(n);
    }
}

double XDemuxThread::Volume() const noexcept(true){
    QMutexLocker locker(const_cast<QMutex*>(&m_mux));
    if (m_at){
        return m_at->Volume();
    }
    return -1.0;
}

void XDemuxThread::SetSpeed(const float &speed) noexcept(true){
    QMutexLocker locker(&m_mux);
    if (!m_at){
        return;
    }

    if (speed <= 0.0f || speed >= 5.0f){
        m_at->SetSpeed(1.0f);
        return;
    }

    m_at->SetSpeed(speed);
}

float XDemuxThread::Speed() const noexcept(true){
    QMutexLocker locker(const_cast<QMutex*>(&m_mux));
    if (m_at){
        return m_at->Speed();
    }
    return -1.0f;
}
