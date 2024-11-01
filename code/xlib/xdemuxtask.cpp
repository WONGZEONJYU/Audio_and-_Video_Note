#include "xdemuxtask.hpp"
#include "xavpacket.hpp"

using namespace std;

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
            XHelper::MSleep(1);
            continue;
        }
        cout << GET_STR(.) << flush;
        Next(pkt);
        XHelper::MSleep(1);
    }
}

XDemuxTask::~XDemuxTask(){
    XThread::Stop();
}
