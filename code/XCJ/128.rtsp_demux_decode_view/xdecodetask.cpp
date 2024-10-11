//
// Created by Fy-WONG on 2024/10/8.
//

#include "xdecodetask.h"
#include "xcodec_parameters.hpp"
#include "xdecode.hpp"
#include "xavpacket.hpp"
#include "xavframe.hpp"

using namespace std::this_thread;
using namespace std::chrono;

void XDecodeTask::Do(XAVPacket &pkt) {
    std::cout << "#";
    if (0 != pkt.stream_index){
        return;
    }
    auto pkt_{new_XAVPacket()};
    *pkt_ = std::move(pkt);
    m_pkt_list.Push(pkt_);
}

void XDecodeTask::Main() {

    {
        std::unique_lock locker(m_mutex);
        if (!m_frame){
            m_frame = new_XAVFrame();
        }
    }

    while (!m_is_exit){
        auto pkt{m_pkt_list.Pop()};
        if (!pkt){
            sleep_for(1ms);
            continue;
        }

        if (!m_decode.Send(*pkt)) {
            sleep_for(1ms);
            continue;
        }

        {
            std::unique_lock locker(m_mutex);
            if (m_decode.Receive(*m_frame)){
                std::cout << "@";
                m_need_view_ = true;
            }
        }
        sleep_for(1ms);
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

XAVFrame_sp XDecodeTask::CopyFrame() {
    std::unique_lock locker(m_mutex);
    if (!m_need_view_ || !m_frame || !m_frame->buf[0]) {
        return {};
    }
    auto f{new_XAVFrame(*m_frame)};
    m_need_view_ = false;
    return f;
}
