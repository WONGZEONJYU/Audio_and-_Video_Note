//
// Created by wong on 2024/9/26.
//

#include <sstream>
#include "xtools.hpp"
#include "xavpacket.hpp"

void XThread::_stop_() {

    m_is_exit = true;
    if (m_th.joinable()) {
        LOGDINFO(GET_STR(thread begin stop));
        m_th.join();
        std::string s(GET_STR(thread end stop :));
        s += std::to_string(m_index);
        LOGDINFO(s);
    }
}

void XThread::Start() {
    static std::atomic_int i{};
    m_is_exit = false;
    std::unique_lock locker(m_mux);
    m_th = std::thread(&XThread::Main, this);
    m_index.store(i++);
    std::stringstream ss;
    ss << GET_STR(thread start index:) << m_index << " ";
    LOGDINFO(ss.str());
}

void XThread::Stop() {
    _stop_();
}

XThread::~XThread() {
    _stop_();
}

XAVPacket_sp XAVPacketList::Pop() {
    std::unique_lock locker(m_mux);
    if (m_packets.empty()){
        return {};
    }
    auto re{m_packets.front()};
    m_packets.pop_front();
    return re;
}

void XAVPacketList::Push(XAVPacket_sp &&pkt) {
    std::unique_lock locker(m_mux);
    m_packets.push_back(std::move(pkt));

    //超出空间,清理数据,到关键帧位置
    if (m_packets.size() > max_packets){

        //如果第一帧是关键帧,清理后直接退出
        if (m_packets.front()->flags & AV_PKT_FLAG_KEY) {
            m_packets.pop_front();
            return;
        }
        //第一帧为非关键帧,则清理整个GOP
        while (!m_packets.empty()){

            if (m_packets.front()->flags & AV_PKT_FLAG_KEY){
                //清理到直到下一组GOP出现才退出
                return;
            }
            m_packets.pop_front();
        }
    }
}

void XAVPacketList::Push(XAVPacket_sp &pkt) {
    Push(std::move(pkt));
}
