//
// Created by wong on 2024/8/10.
//

#include "XDemuxThread.hpp"

XDemuxThread::XDemuxThread(std::exception_ptr * e):
QThread(), m_ex_ptr(e){

}

XDemuxThread::~XDemuxThread() {
    m_is_exit = true;
    quit();
    wait();
}

void XDemuxThread::Open() noexcept(false) {

}

void XDemuxThread::run() {

    QMutexLocker locker(&m_re_mux);

    try {
        while (!m_is_exit){
            if (!m_demux){
                locker.unlock();
                msleep(1);
                locker.relock();
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
