//
// Created by Administrator on 2024/8/9.
//
#include "XAVQThreadAbstract.hpp"
#include "XHelper.hpp"

XAVQThreadAbstract::XAVQThreadAbstract(std::exception_ptr *e):
QThread(),m_exceptionPtr{e} {
}

XAVQThreadAbstract::~XAVQThreadAbstract() {
    Exit_Thread();
}

void XAVQThreadAbstract::Push(XAVPacket_sptr &&pkt) noexcept(false) {

    if (!pkt){
        PRINT_ERR_TIPS(GET_STR(XAVPacket_sptr is empty));
        return;
    }

    while (!m_is_Exit) {
        QMutexLocker lock(&m_mux);
        if (m_Packets.size() < Max_List){
            m_Packets.enqueue(pkt);
            m_wc.wakeAll();
            break;
        }
        m_wc.wait(&m_mux,1);
    }
}

void XAVQThreadAbstract::Exit_Thread() noexcept(true) {
    m_is_Exit = true;
    m_wc.wakeAll();
    quit();
    wait();
}
