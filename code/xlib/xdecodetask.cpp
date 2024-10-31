//
// Created by Fy-WONG on 2024/10/8.
//

#include "xdecodetask.hpp"
#include "xcodec_parameters.hpp"
#include "xdecode.hpp"
#include "xavpacket.hpp"
#include "xavframe.hpp"

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

void XDecodeTask::Do(XAVPacket &pkt) {

    if (0 != pkt.stream_index){
        return;
    }

    if(const auto pkt_{new_XAVPacket(pkt)}) {
        m_pkt_list_.Push(std::move(pkt));
        cout << "#" << flush;
    }
}

void XDecodeTask::Main() {

    {
        unique_lock locker(m_mutex_);
        if (!m_frame_){
            if (!((m_frame_ = new_XAVFrame()))) {
                while (!m_is_exit_) {
                    PRINT_ERR_TIPS(GET_STR(new_XAVFrame error!));
                    sleep_for(1ms);
                }
            }
        }
    }

    while (!m_is_exit_){
        const auto pkt{m_pkt_list_.Pop()};
        if (!pkt){
            sleep_for(1ms);
            continue;
        }

        if (!m_decode_.Send(*pkt)) {
            sleep_for(1ms);
            continue;
        }

        {
            unique_lock locker(m_mutex_);
            if (m_decode_.Receive(*m_frame_)){
                std::cout << "@";
                m_need_view_ = true;
            }
        }
        sleep_for(1ms);
    }
}

bool XDecodeTask::Open(const XCodecParameters_sp &parm) {

    const auto c{XDecode::Create(parm->Codec_id(),false)};

    if (!c) {
        LOGERROR(GET_STR(Decode::Create failed!));
        return {};
    }

    parm->to_context(c);
    unique_lock locker(m_mutex_);
    m_decode_.set_codec_ctx(c);
    IS_FALSE_(m_decode_.Open(),return {});
    LOGDINFO(GET_STR(Open codec success!));
    return true;
}

XAVFrame_sp XDecodeTask::CopyFrame() {
    unique_lock locker(m_mutex_);
    if (!m_need_view_ || !m_frame_ || !m_frame_->buf[0]) {
        return {};
    }
    auto f{new_XAVFrame(*m_frame_)};
    m_need_view_ = false;
    return f;
}
