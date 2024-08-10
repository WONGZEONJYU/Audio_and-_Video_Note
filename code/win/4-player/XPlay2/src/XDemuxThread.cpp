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

XDemuxThread::XDemuxThread(std::exception_ptr * e):
QThread(), m_ex_ptr(e){

}

XDemuxThread::~XDemuxThread() {
    DeConstruct();
}

void XDemuxThread::DeConstruct() noexcept(true) {
    m_is_exit = true;
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
        m_demux.reset(new XDemux());
        m_demux->Open(url.toStdString());
        m_ac = m_demux->Copy_Present_AudioCodecParam();
        m_vc = m_demux->Copy_Present_VideoCodecParam();

        m_at.reset(new XAudioThread(m_ex_ptr.load()));
        m_at->Open(m_ac);

        m_vt.reset(new XVideoThread(m_ex_ptr.load()));
        m_vt->Open(m_vc,call);

        m_wc.wakeAll();

    } catch (...) {
        locker.unlock();
        throw;
    }
}

void XDemuxThread::run() {

    try {
        while (!m_is_exit){

            QMutexLocker locker(&m_mux);

            if (!m_demux || !m_at || !m_vt){
                m_wc.wait(&m_mux,1);
                continue;
            }

            XAVPacket_sptr pkt;
            CHECK_EXC(pkt = m_demux->Read(),locker.unlock()); //可能有异常
            if (!pkt){
                m_wc.wait(&m_mux,5);
                continue;
            }

            const auto a_index {m_demux->Present_Audio_Index()},
                        v_index {m_demux->Present_Video_Index()};

            if (a_index == pkt->stream_index){
                m_at->Push(std::move(pkt));
            }else if (v_index == pkt->stream_index){
                m_vt->Push(std::move(pkt));
            } else{
                pkt.reset();
            }
        }

    } catch (...) {
        QMutexLocker locker(&m_mux);
        if (m_ex_ptr){
            *m_ex_ptr = std::current_exception();
        }
    }
}

void XDemuxThread::Start() noexcept(false) {
    QMutexLocker locker(&m_mux);
    if (!m_demux || !m_at || !m_vt) {
        PRINT_ERR_TIPS(GET_STR(Please initialize first));
        return;
    }
    start();
    m_at->start();
    m_vt->start();
}
