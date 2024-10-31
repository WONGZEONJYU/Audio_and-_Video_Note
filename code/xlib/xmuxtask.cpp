#include "xmuxtask.hpp"
#include "xmux.hpp"
#include "xcodec_parameters.hpp"

void XMuxTask::Do(XAVPacket &pkt){

    m_pkts_.Push(pkt);
    Next(pkt);
}

void XMuxTask::Main() {
    m_xmux_.WriteHead();
    while (!m_is_exit_) {
        std::unique_lock locker(m_mux_);
        auto pkt{m_pkts_.Pop()};
        if (!pkt){
            XHelper::MSleep(1);
            continue;
        }
        m_xmux_.Write(*pkt);
        std::cout << "W";
    }
    m_xmux_.WriteEnd();
}

bool XMuxTask::Open(const std::string &url,
                    const XCodecParameters &video_parm,
                    const XCodecParameters &audio_parm) {

    const auto c{XMux::Open(url,video_parm,audio_parm)};
    if (!c){
        return {};
    }

    m_xmux_.set_fmt_ctx(c);

    if (video_parm.Video_pixel_format() >= 0){
        m_xmux_.set_video_time_base(video_parm.time_base());
    }

    if (audio_parm.Audio_sample_format() >= 0){
        m_xmux_.set_audio_time_base(audio_parm.time_base());
    }

    return true;
}
