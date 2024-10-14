//
// Created by wong on 2024/9/27.
//

#include "xdemuxtask.hpp"
#include <chrono>
#include "xavpacket.hpp"

using namespace std::chrono;

bool XDemuxTask::Open(const std::string &url, const uint64_t &time_out) {

    m_url_ = url;
    m_timeout_ms_ = time_out;
    m_demux_.set_fmt_ctx({});
    const auto c{XDemux::Open(url)};
    if (!c){
        return {};
    }
    m_demux_.set_fmt_ctx(c);
    m_demux_.set_timeout_ms(time_out);
    return true;
}

void XDemuxTask::Main() {

    XAVPacket pkt;
    while (!m_is_exit_) {
        if (!m_demux_.Read(pkt)){
            std::cout << GET_STR(-);
            if (!m_demux_.is_connected()){
                Open(m_url_,m_timeout_ms_);
            }
            std::this_thread::sleep_for(1ms);
            continue;
        }
        std::cout << GET_STR(.);
        Next(pkt);
        std::this_thread::sleep_for(1ms);
    }
}
