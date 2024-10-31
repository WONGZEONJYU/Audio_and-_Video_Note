//
// Created by wong on 2024/9/27.
//

#include "xdemuxtask.hpp"
#include <chrono>
#include "xavpacket.hpp"

using namespace std;
using namespace std::this_thread;
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
            cout << GET_STR(-) << flush;
            if (!m_demux_.is_connected()){
                Open(m_url_,m_timeout_ms_);
            }
            sleep_for(1ms);
            continue;
        }
        cout << GET_STR(.) << flush;
        Next(pkt);
        sleep_for(1ms);
    }
}
