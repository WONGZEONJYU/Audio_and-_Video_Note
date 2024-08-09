//
// Created by Administrator on 2024/8/9.
//
#include "XAVQThread_Abstract.hpp"
#include "XHelper.hpp"

XAVQThread_Abstract::XAVQThread_Abstract(std::exception_ptr *e):
QThread(),m_exceptionPtr{e} {

}

XAVQThread_Abstract::~XAVQThread_Abstract() {
    Exit_Thread();
}

void XAVQThread_Abstract::Push(XAVPacket_sptr &&pkt) noexcept(false){
    if (!pkt){
        PRINT_ERR_TIPS(GET_STR(XAVPacket_sptr is empty));
        return;
    }

    while (!m_is_Exit) {
        {
            QMutexLocker lock(&m_re_mux);
            if (m_Packets.size() < Max_List){
                m_Packets.enqueue(pkt);
                break;
            }
        }
        msleep(1);
    }
}

void XAVQThread_Abstract::Exit_Thread() noexcept(true) {
    m_is_Exit = true;
    quit();
    wait();
}

void XAVQThread_Abstract::SetException_ptr(std::exception_ptr *e) noexcept(true){
    m_exceptionPtr = e;
}
