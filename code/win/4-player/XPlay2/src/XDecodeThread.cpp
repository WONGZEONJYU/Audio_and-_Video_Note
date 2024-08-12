//
// Created by Administrator on 2024/8/9.
//
#include "XDecodeThread.hpp"
#include "XHelper.hpp"
#include "XDecode.hpp"

XDecodeThread::XDecodeThread(std::exception_ptr *e):
QThread(),m_exceptionPtr{e} {
    if (!m_decode){
        m_decode.reset(new XDecode());
    }
}

XDecodeThread::~XDecodeThread(){
    Exit_Thread();
}

void XDecodeThread::Push(XAVPacket_sptr &&pkt) noexcept(false) {

    if (!pkt){
        PRINT_ERR_TIPS(GET_STR(XAVPacket_sptr is empty));
        return;
    }

    while (!m_is_Exit) {
        QMutexLocker lock(&m_d_mux);
        if (m_Packets.size() < Max_List){
            m_Packets.push_back(std::move(pkt));
            //m_cv.wakeAll();
            break;
        }
        lock.unlock();
        msleep(1);
        //m_cv.wait(&m_d_mux,1);
    }
}

void XDecodeThread::Clear() noexcept(true) {
    QMutexLocker lock(&m_d_mux);
    m_decode->Clear();
    m_Packets.clear();
}

void XDecodeThread::Close() noexcept(true) {
    Clear();
    Exit_Thread();
    m_decode->Close();
    QMutexLocker lock(&m_d_mux);
    m_decode.reset();
}

void XDecodeThread::Exit_Thread() noexcept(true) {
    m_is_Exit = true;
    //m_cv.wakeAll();
    quit();
    wait();
}

void XDecodeThread::run() noexcept(false){
    entry();
}

XAVPacket_sptr XDecodeThread::Pop() noexcept(false) {

    QMutexLocker lock(&m_d_mux);
    if (m_Packets.isEmpty()){
        return {};
    }
    return m_Packets.first();
}

void XDecodeThread::PopFront() noexcept(false) {
    QMutexLocker lock(&m_d_mux);
    if (m_Packets.isEmpty()){
        return;
    }
    m_Packets.removeFirst();
    //m_cv.wakeAll();
}

void XDecodeThread::Create_Decode() noexcept(false) {
//    QMutexLocker lock(&m_d_mux);
//    if (!m_decode){
//        m_decode.reset(new XDecode());
//    }
}
