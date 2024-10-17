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
                    const XCodecParameters_sp &video_parm,
                    const XCodecParameters_sp &audio_parm) {

    auto c{XMux::Open(url,video_parm,audio_parm)};
    if (!c){
        return {};
    }

    m_xmux_.set_fmt_ctx(c);

    if (video_parm){
        const auto tb{video_parm->time_base()};
        m_xmux_.set_video_time_base(&tb);
    }

    if (audio_parm){
        const auto tb{audio_parm->time_base()};
        m_xmux_.set_audio_time_base(&tb);
    }

    return true;
}
