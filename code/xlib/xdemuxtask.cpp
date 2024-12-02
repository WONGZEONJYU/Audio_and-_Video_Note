#include "xdemuxtask.hpp"
#include "xavpacket.hpp"
#include "xcodec_parameters.hpp"

using namespace std;

bool XDemuxTask::Open(const std::string &url, const uint64_t &time_out) {
    m_is_open_ = false;
    m_url_ = url;
    m_timeout_ms_ = time_out;
    m_demux_.set_fmt_ctx({});
    AVFormatContext *c{};
    IS_NULLPTR(c = XDemux::Open(url),return {});
    m_demux_.set_fmt_ctx(c);
    m_demux_.set_timeout_ms(time_out);
    m_is_open_ = true;
    return true;
}

void XDemuxTask::Main() {

    while (!is_exit()) {

        if (is_pause()){
            MSleep(1);
            continue;
        }

        XAVPacket pkt;
        if (!m_demux_.Read(pkt)){
            cout << GET_STR(N) << flush;
            if (!m_demux_.is_connected()){
                Open(m_url_,m_timeout_ms_);
            }
            MSleep(1);
            continue;
        }

        //cout << GET_STR(R) << flush;

        if (m_demux_.video_index() == pkt.stream_index &&
            SYNC_VIDEO == m_sync_type_) { //用于没有音频时候,视频同步
            auto dur{m_demux_.RescaleToMs(pkt.duration,pkt.stream_index)};
            if (dur < 0) {
                dur = 40;
            }
            dur = static_cast<decltype(dur)>(static_cast<double>(dur) / m_speed_);
            MSleep(dur);
        }

        Next(pkt);
        MSleep(1);
    }
}

XDemuxTask::~XDemuxTask(){
    cerr << "begin " <<__FUNCTION__ << " current thread_id = " << XHelper::present_thread_id() << "\n";
    Stop();
    cerr << "begin " <<__FUNCTION__ << " current thread_id = " << XHelper::present_thread_id() << "\n";
}

bool XDemuxTask::Seek(const int64_t &ms) {

    const auto vp{m_demux_.CopyVideoParm()};
    if (!vp){
        return {};
    }

    const auto pts{XHelper::XRescale(ms,{1,1000},vp->time_base())};
    return m_demux_.Seek(pts,video_index());
}

bool XDemuxTask::ReadVideoPacket(XAVPacket &pkt) {

    const auto v_inx{m_demux_.video_index()};
    if (v_inx < 0) {
        return {};
    }

    for (int i {}; i < 25 ;++i) {
        if (!m_demux_.Read(pkt)) {
            return {};
        }

        if (v_inx == pkt.stream_index) {
            return true;
        }
    }
    return {};
}

void XDemuxTask::Stop() {
    XThread::Stop();
    m_is_open_ = false;
}

void XDemuxTask::Clear() {
    m_demux_.Clear();
}
