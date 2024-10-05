//
// Created by wong on 2024/9/27.
//

#include "xdemuxtask.hpp"
#include <chrono>
#include "xavpacket.hpp"

using namespace std::chrono;

bool XDemuxTask::Open(const std::string &url, const uint64_t &time_out) {

    LOGDEBUG(GET_STR(begin!));
    m_url = url;
    m_timeout_ms = time_out;
    m_demux.set_fmt_ctx({});
    auto c{XDemux::Open(url)};
    if (!c){
        return {};
    }
    m_demux.set_fmt_ctx(c);
    m_demux.set_timeout_ms(time_out);
    LOGDEBUG(GET_STR(end!));
    return true;
}

void XDemuxTask::Main() {

    XAVPacket pkt;
    while (!m_is_exit){
        if (!m_demux.Read(pkt)){
            std::cout << GET_STR(-);
            if (!m_demux.is_connected()){
                Open(m_url,m_timeout_ms);
            }

            std::this_thread::sleep_for(1ms);
            continue;
        }
        std::cout << GET_STR(.);
        std::this_thread::sleep_for(1ms);
    }
}
