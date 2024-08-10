//
// Created by wong on 2024/8/10.
//
#include <QDebug>
#include "XDemuxThread.hpp"
#include "XAudioThread.hpp"
#include "XVideoThread.hpp"
#include "XAVCodecParameters.hpp"
#include "XDemux.hpp"

XDemuxThread::XDemuxThread(std::exception_ptr * e):
QThread(), m_ex_ptr(e){

}

XDemuxThread::~XDemuxThread() {
    m_is_exit = true;
    quit();
    wait();
}

void XDemuxThread::Open(const QString &url,IVideoCall *call) noexcept(false) {

    if (url.isEmpty()){
        PRINT_ERR_TIPS(GET_STR(url is empty));
        return;
    }

    if (call){
        PRINT_ERR_TIPS(GET_STR(IVideoCall is empty));
        return;
    }

    QMutexLocker locker(&m_mux);

    try {

        m_demux.reset(new XDemux);
        


    } catch (...) {
        locker.unlock();
        throw ;
    }
}

void XDemuxThread::run() {

    QMutexLocker locker(&m_mux);

    try {
        while (!m_is_exit){
            if (!m_demux || !m_at || !m_vt){
                m_wc.wait(&m_mux,1);
                continue;
            }

        }

    } catch (...) {
        locker.unlock();
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
