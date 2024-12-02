#include "xmuxtask.hpp"
#include "xmux.hpp"
#include "xcodec_parameters.hpp"
#include "xavpacket.hpp"
#include <sstream>

using namespace std;

void XMuxTask::Do(XAVPacket &pkt){
    if (m_pkts_.Push(pkt)) {
        cout << " mux " <<
            GET_STR(push index:) << " " <<
                pkt.stream_index << " " << flush;
    }
    Next(pkt);
}

void XMuxTask::Main() {
    m_xmux_.WriteHead();

    while (!is_exit()) {
        unique_lock locker(m_mux_);
        const auto pkt{m_pkts_.Pop()};
        if (!pkt){
            locker.unlock();
            MSleep(1);
            continue;
        }

        if (m_xmux_.video_index() == pkt->stream_index
            && pkt->flags & AV_PKT_FLAG_KEY) {
            m_xmux_.Write(*pkt);
            cout << GET_STR(W) << flush;
            break;
        }
    }

    while (!is_exit()) {
        unique_lock locker(m_mux_);
        const auto pkt{m_pkts_.Pop()};
        if (!pkt){
            locker.unlock();
            XHelper::MSleep(1);
            continue;
        }
        m_xmux_.Write(*pkt);
        cout << GET_STR(W) << flush;
    }

    m_xmux_.WriteEnd();
}

bool XMuxTask::Open(const std::string &url,
                    const XCodecParameters &video_parm,
                    const XCodecParameters &audio_parm) {
    AVFormatContext *c{};
    IS_NULLPTR(c = XMux::Open(url,video_parm,audio_parm),return {});
    m_xmux_.set_fmt_ctx(c);

    if (video_parm.Video_pixel_format() >= 0){
        m_xmux_.set_video_time_base(video_parm.time_base());
    }

    if (audio_parm.Audio_sample_format() >= 0){
        m_xmux_.set_audio_time_base(audio_parm.time_base());
    }

    return true;
}

bool XMuxTask::Open(const std::string &url,
    const XCodecParameters *video_parm,
    const XCodecParameters *audio_parm) {

    return Open(url, video_parm ? *video_parm : XCodecParameters(),
        audio_parm ? *audio_parm : XCodecParameters());
}

XMuxTask::~XMuxTask() {
    XThread::Stop();
}
