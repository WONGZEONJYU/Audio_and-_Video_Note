#include "xdemuxtask.hpp"
#include "xavpacket.hpp"

using namespace std;

bool XDemuxTask::Open(const std::string &url, const uint64_t &time_out) {
    m_url_ = url;
    m_timeout_ms_ = time_out;
    m_demux_.set_fmt_ctx({});
    AVFormatContext *c{};
    IS_NULLPTR(c = XDemux::Open(url),return {});
    m_demux_.set_fmt_ctx(c);
    m_demux_.set_timeout_ms(time_out);
    return true;
}

void XDemuxTask::Main() {

    while (!m_is_exit_) {
        XAVPacket pkt;
        if (!m_demux_.Read(pkt)){
            cout << GET_STR(N) << flush;
            if (!m_demux_.is_connected()){
                Open(m_url_,m_timeout_ms_);
            }
            XHelper::MSleep(1);
            continue;
        }

        cout << GET_STR(R) << flush;

        if (m_demux_.video_index() == pkt.stream_index &&
            SYNC_VIDEO == m_sync_type_) { //用于没有音频时候,视频同步
            auto dur{m_demux_.RescaleToMs(pkt.duration,pkt.stream_index)};
            if (dur < 0) {
                dur = 40;
            }
            dur = static_cast<decltype(dur)>(static_cast<double>(dur) / m_speed_);
            XHelper::MSleep(dur);
        }

        Next(pkt);
        XHelper::MSleep(1);
    }
}

XDemuxTask::~XDemuxTask(){
    cerr << "begin " <<__FUNCTION__ << " current thread_id = " << XHelper::present_thread_id() << "\n";
    XThread::Stop();
    cerr << "begin " <<__FUNCTION__ << " current thread_id = " << XHelper::present_thread_id() << "\n";
}
