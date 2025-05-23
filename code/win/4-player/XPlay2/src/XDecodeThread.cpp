//
// Created by Administrator on 2024/8/9.
//
#include "XDecodeThread.hpp"
#include "XHelper.hpp"
#include "XDecode.hpp"
#include "XAVFrame.hpp"
#include <QDebug>

XDecodeThread::XDecodeThread(std::exception_ptr *e):
QThread(),m_exceptionPtr{e} {

    try {
        Create_Decode();
        m_Packets.reserve(Max_List);
    } catch (const std::exception &e) {
        qDebug() << e.what();
    }
}

XDecodeThread::~XDecodeThread() {
    Exit_Thread();
}

void XDecodeThread::Create_Decode() noexcept(false) {
    QMutexLocker locker(&m_d_mux);
    if (!m_decode){
        CHECK_EXC(m_decode.reset(new XDecode()),locker.unlock());
    }
}

void XDecodeThread::Open(const XAVCodecParameters_sptr &p) {
    Create_Decode();
    Clear();
    m_decode->Open(p);
    //m_d_cv.wakeAll();
}

void XDecodeThread::Clear() noexcept(true) {
    m_pts = m_sync_pts = 0;
    m_speed = 1.0f;
    m_is_Pause = true;
    if (m_decode){
        m_decode->Clear();
    }
    QWriteLocker locker(&m_rw_mux);
    m_Packets.clear();
}

void XDecodeThread::Close() noexcept(true) {
    Clear();
    Exit_Thread();
    if (m_decode) {
        m_decode->Close();
    }
    QMutexLocker locker(&m_d_mux);
    m_decode.reset();
}

void XDecodeThread::Exit_Thread() noexcept(true) {
    m_is_Exit = true;
    //m_d_cv.wakeAll();
    quit();
    wait();
}

void XDecodeThread::run() noexcept(false){
    entry();
}

bool XDecodeThread::Push(XAVPacket_sptr &&pkt) noexcept(false) {
    /**
     * pkt允许为空,用于冲刷解码器
     */
    bool b{};
    QWriteLocker locker(&m_rw_mux);
    if (m_Packets.size() < Max_List){
        m_Packets.emplaceBack(std::move(pkt));
        b = !b;
    }
    return b;
}

XAVPacket_sptr XDecodeThread::Pop() noexcept(false) {
    QReadLocker locker(&m_rw_mux);
    if (m_Packets.isEmpty()){
        return {};
    }
    return m_Packets.first();
}

void XDecodeThread::PopFront() noexcept(false) {
    QWriteLocker locker(&m_rw_mux);
    if (m_Packets.isEmpty()) {
        return;
    }
    m_Packets.removeFirst();
    //m_d_cv.wakeAll();
}

bool XDecodeThread::Empty() const noexcept(false){
    QReadLocker locker(const_cast<decltype(m_rw_mux)*>(&m_rw_mux));
    return m_Packets.isEmpty();
}

bool XDecodeThread::Send_Packet(const XAVPacket_sptr &pkt) const noexcept(false) {
    bool b{};
    QMutexLocker locker(const_cast<decltype(m_d_mux)*>(&m_d_mux));
    if (m_decode){
        CHECK_EXC(b = m_decode->Send(pkt),locker.unlock());
    }
    return b;
}

bool XDecodeThread::Send_Packet() noexcept(false) {
    auto b {!Empty()};
    if (b) {
        b = Send_Packet(Pop());
        if (b){
            PopFront();
        }
    }
    return b;
}

XAVFrame_sptr XDecodeThread::Receive_Frame(int64_t &pts) noexcept(false) {
    QMutexLocker locker(&m_d_mux);
    if (m_decode){
        XAVFrame_sptr frame;
        CHECK_EXC(frame = m_decode->Receive(),locker.unlock());
        pts = m_decode->Pts();
        return frame;
    }
    return {};
}
