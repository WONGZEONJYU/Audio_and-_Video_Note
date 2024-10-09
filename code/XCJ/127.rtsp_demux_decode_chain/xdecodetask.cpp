//
// Created by Fy-WONG on 2024/10/8.
//

#include "xdecodetask.h"
#include "xcodec_parameters.hpp"
#include "xdecode.hpp"
#include "xavpacket.hpp"

void XDecodeTask::Main() {

}

void XDecodeTask::Do(XAVPacket &pkt) {
    std::cout << "#";
    auto pktsp = new_XAVPacket();
    *pktsp = std::move(pkt);

    if (0 == pktsp->stream_index) {
        std::cout << GET_STR(pktsp->size :) << pktsp->size << "\n";
        std::cout << GET_STR(pkt.pts :) << pkt.pts << "\n";
    }
}

bool XDecodeTask::Open(const XCodecParameters &parms) {

    const auto c{XDecode::Create(parms.Codec_id(),false)};

    if (!c) {
        LOGERROR(GET_STR(Decode::Create failed!));
        return {};
    }

    parms.to_context(c);
    std::unique_lock locker(m_mutex);
    m_decode.set_codec_ctx(c);

    if (!m_decode.Open()) {
        LOGERROR(GET_STR(Decode::Open failed!));
        return {};
    }
    LOGDINFO(GET_STR(Open codec success!));
    return true;
}
