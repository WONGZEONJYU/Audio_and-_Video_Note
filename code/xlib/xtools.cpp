#include <sstream>
#include "xtools.hpp"
#include "xavpacket.hpp"
extern "C" {
#include <libavutil/mathematics.h>
}

using namespace std;

void XThread::MSleep(const uint64_t &ms) {
    using XHelper::Get_time_ms;
    using std::this_thread::sleep_for;

    const auto begin{Get_time_ms()};
    auto ms_{ms};
    while (ms_--) {
        sleep_for(1ms);
        if (Get_time_ms() - begin >= ms){
            return;
        }
    }
}

void XThread::_stop_() {
    stringstream ss;
    ss << GET_STR(thread index:) << " "<<  m_index_ << " " << GET_STR(begin stop!) << " ";
    LOG_INFO(ss.str());
    ss.clear();
    m_is_exit_ = true;
    m_next_ = {};
    ss << GET_STR(thread index:) << " "<<  m_index_ << " " << GET_STR(end stop!) << " ";
    LOG_INFO(ss.str());
}

void XThread::_wait_() {
    if (m_th_.joinable()) {
        stringstream ss;
        ss << GET_STR(thread index:) << " "<< m_index_ << " " << GET_STR(begin wait!) << " ";
        LOG_INFO(ss.str());
        ss.clear();
        m_th_.join();
        ss << GET_STR(thread index:) << " "<< m_index_ << " " << GET_STR(end wait!) << " ";
        LOG_INFO(ss.str());
    }
}

void XThread::Start() {
    static atomic_int i{};
    m_is_exit_ = false;
    unique_lock locker(m_mux_);
    m_th_ = thread(&XThread::Main, this);
    m_index_ = i++;
    stringstream ss;
    ss << GET_STR(thread start index:) << " " << m_index_ << " ";
    LOG_INFO(ss.str());
}

void XThread::Stop() {
    _stop_();
}

void XThread::Wait() {
    _wait_();
}

XThread::~XThread() {
    _stop_();
    _wait_();
}

XAVPacket_sp XAVPacketList::Pop() {
    unique_lock locker(m_mux_);
    if (m_packets_.empty()){
        return {};
    }
    const auto re{m_packets_.front()};
    m_packets_.pop_front();
    return re;
}

void XAVPacketList::Push(XAVPacket_sp &&pkt) {
    unique_lock locker(m_mux_);
    //m_packets_.push_back(std::move(pkt));
    m_packets_.emplace_back(std::move(pkt));
    //超出空间,清理数据,到关键帧位置
    if (m_packets_.size() > max_packets){

        //如果第一帧是关键帧,清理后直接退出
        if (m_packets_.front()->flags & AV_PKT_FLAG_KEY) {
            m_packets_.pop_front();
            return;
        }
        //第一帧为非关键帧,则清理整个GOP
        while (!m_packets_.empty()){

            if (m_packets_.front()->flags & AV_PKT_FLAG_KEY){
                //清理到直到下一组GOP出现才退出
                return;
            }
            m_packets_.pop_front();
        }
    }
}

void XAVPacketList::Push(const XAVPacket_sp &pkt){
    auto pkt_{pkt};
    Push(std::move(pkt_));
}

bool XAVPacketList::Push(const XAVPacket &pkt){
    XAVPacket_sp pkt_;
    IS_SMART_NULLPTR(pkt_ = new_XAVPacket(pkt), return {});
    Push(std::move(pkt_));
    return true;
}

bool XAVPacketList::Push(XAVPacket &&pkt){
    XAVPacket_sp pkt_;
    IS_SMART_NULLPTR(pkt_ = new_XAVPacket(), return {});
    *pkt_ = std::move(pkt);
    Push(std::move(pkt_));
    return true;
}

uint64_t XAVPacketList::Size() const {
    unique_lock locker(const_cast<decltype(m_mux_)&>(m_mux_));
    return m_packets_.size();
}

void XAVPacketList::Clear() {
    unique_lock locker(m_mux_);
    m_packets_.clear();
}
