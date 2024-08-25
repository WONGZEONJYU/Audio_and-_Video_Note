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
    //m_cv.wakeAll();
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

        m_vt->HasAudio(m_ac.operator bool());

        m_total_Ms = m_demux->totalMS();
        //m_cv.wakeAll();

    } catch (...) {
        locker.unlock();
        throw;
    }
}

void XDemuxThread::Push_helper(XAVPacket_sptr &&pkt) noexcept(false){

    bool is_flush{};
    while (!m_is_Exit){

        if (!pkt){ //已经读取不到数据,冲刷解码器
            QMutexLocker locker(&m_mux);
            const auto a_index {m_demux->Present_Audio_Index()},
                    v_index {m_demux->Present_Video_Index()};

            if (a_index >= 0 && !is_flush){
                if (!m_at->Push(std::move(pkt))){
                    locker.unlock();
                    msleep(1);
                    continue;
                }
                is_flush = true;
            }

            if (v_index >= 0){
                if (!m_vt->Push(std::move(pkt))){
                    locker.unlock();
                    msleep(1);
                    continue;
                }
            }
        }else {
            const auto index{pkt->stream_index};
            QMutexLocker locker(&m_mux);
            const auto a_index {m_demux->Present_Audio_Index()},
                    v_index {m_demux->Present_Video_Index()};

            if (a_index == index){
                if (!m_at->Push(std::move(pkt))){
                    locker.unlock();
                    msleep(1);
                    continue;
                }
            }else if (v_index == index){
                if (!m_vt->Push(std::move(pkt))){
                    locker.unlock();
                    msleep(1);
                    continue;
                }
            } else{}
        }
        break;
    }
    msleep(1);
}

void XDemuxThread::run() {

    try {
        bool end{};
        while (!m_is_Exit){

            if (m_isPause){
                msleep(5);
                continue;
            }

            XAVPacket_sptr pkt;
            {
                QMutexLocker locker(&m_mux);
                /* 音频获取到到pts给视频进行同步*/
                m_vt->Set_Sync_Pts(m_at->Pts());
                /*用于进度条*/
                m_pts = m_at ? m_at->Pts() : m_vt->Pts();
                CHECK_EXC(pkt = m_demux->Read(),locker.unlock()); //可能有异常
            }

            if (!pkt && !end){
                end = true;
                Push_helper({});
            }else if (!pkt) {
                msleep(1);
            }else{
                end = false;
                Push_helper(std::move(pkt));
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

void XDemuxThread::Seek(const double &pos) noexcept(false) {

    Clear();
    const auto status {m_isPause.load()};
    SetPause(true); //无论当前什么状态,都先暂停再seek

    QMutexLocker locker(&m_mux);
    if (m_demux){
        m_demux->Seek(pos);
        /*先seek到指定位置*/
    }

    const auto t_pos {pos * static_cast<double>(m_total_Ms.load())};
    const auto seek_pts{static_cast<int64_t>(t_pos)};

    while (!m_is_Exit) {
        XAVPacket_sptr pkt;
        CHECK_EXC(pkt = m_demux->ReadVideo(),locker.unlock());
        /*先读取一次AVPacket*/
        if (!pkt) {
            break;
        }
        if (m_vt->RepaintPts(pkt,seek_pts)) { //解码并显示当前帧
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

void XDemuxThread::SetSpeed(float speed) noexcept(true){
    QMutexLocker locker(&m_mux);
    if (!m_at || !m_vt){
        return;
    }

    m_at->SetSpeed(speed);
    m_vt->SetSpeed(speed);
}

float XDemuxThread::Speed() const noexcept(true){

    QMutexLocker locker(const_cast<QMutex*>(&m_mux));
    if (m_at){
        return m_at->Speed();
    }

    if (m_vt){
        return m_vt->Speed();
    }

    return -1.0f;
}
