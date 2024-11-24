#include <sstream>
#include "xtools.hpp"
#include "xavpacket.hpp"
extern "C" {
#include <libavutil/mathematics.h>
}

auto XRescale(const int64_t &pts,
    const AVRational &src_tb,
    const AVRational &dst_tb) ->int64_t {
    return av_rescale_q(pts, src_tb, dst_tb);
}

auto XRescale(const int64_t &pts,
    const XRational &src_tb,
    const XRational &dst_tb) ->int64_t {

    const AVRational src{.num = src_tb.num,.den = src_tb.den};
    const AVRational dst{.num = dst_tb.num,.den = dst_tb.den};
    return XRescale(pts, src, dst);
}

void XThread::_stop_() {
    m_is_exit_ = true;
    m_next_ = {};
    if (m_th_.joinable()) {
        LOGDINFO(GET_STR(thread begin stop));
        m_th_.join();
        std::stringstream ss;
        ss << GET_STR(thread end stop index:) << m_index_ << " ";
        LOGDINFO(ss.str());
    }
}

void XThread::Start() {
    static std::atomic_int i{};
    m_is_exit_ = false;
    std::unique_lock locker(m_mux_);
    m_th_ = std::thread(&XThread::Main, this);
    m_index_ = i++;
    std::stringstream ss;
    ss << GET_STR(thread start index:) << m_index_ << " ";
    LOGDINFO(ss.str());
}

void XThread::Stop() {
    _stop_();
}

XThread::~XThread() {
    _stop_();
}

XAVPacket_sp XAVPacketList::Pop() {
    std::unique_lock locker(m_mux_);
    if (m_packets_.empty()){
        return {};
    }
    auto re{m_packets_.front()};
    m_packets_.pop_front();
    return re;
}

void XAVPacketList::Push(XAVPacket_sp &&pkt) {
    std::unique_lock locker(m_mux_);
    m_packets_.push_back(std::move(pkt));

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
    std::unique_lock locker(const_cast<decltype(m_mux_)&>(m_mux_));
    return m_packets_.size();
}
