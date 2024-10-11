//
// Created by Fy-WONG on 2024/10/11.
//

#include "xmuxtask.h"
#include "xmux.hpp"
#include "xcodec_parameters.hpp"

void XMuxTask::Main() {

    while (!m_is_exit) {

    }
}

bool XMuxTask::Open(const std::string &url,
                    const XCodecParameters *video_parm,
                    const AVRational *video_tb,
                    const XCodecParameters *audio_parm,
                    const AVRational *audio_tb) {

    auto c{XMux::Open(url)};
    if (!c){
        return {};
    }

    return false;
}
