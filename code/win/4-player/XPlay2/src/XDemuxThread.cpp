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
    QThread(), m_ex_ptr(e) {

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

    } catch (...) {
        locker.unlock();
        throw;
    }
}

void XDemuxThread::run() {

    try {
        bool end{};
        while (!m_is_exit){

            QMutexLocker locker(&m_mux);

            const auto a_index {m_demux->Present_Audio_Index()},
                    v_index {m_demux->Present_Video_Index()};
            /**
             * 音频获取到到pts给视频进行同步
             */
            m_vt->Set_Sync_Pts(m_at->Pts());

            XAVPacket_sptr pkt;
            CHECK_EXC(pkt = m_demux->Read(),locker.unlock()); //可能有异常
            if (!pkt && !end){
                end = true;
                if (a_index >= 0){
                    m_at->Push({});
                }
                if (v_index >= 0){
                    m_vt->Push({});
                }
                locker.unlock();
                msleep(1);
                continue;
            }
            else if (!pkt) {
                locker.unlock();
                msleep(5);
                continue;
            }else{
                end = false;
            }

            const auto  pkt_index {pkt->stream_index};

            if (a_index == pkt_index){
                m_at->Push(std::move(pkt));
            }else if (v_index == pkt_index){
                m_vt->Push(std::move(pkt));
            } else{
                pkt.reset();
            }

            locker.unlock();
            usleep(1);
        }

    } catch (...) {
        if (m_ex_ptr){
            *m_ex_ptr = std::current_exception();
        }
    }
    qDebug() << "XDemuxThread::run exit";
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
