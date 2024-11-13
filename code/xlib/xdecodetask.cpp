#include "xdecodetask.hpp"
#include "xcodec_parameters.hpp"
#include "xdecode.hpp"
#include "xavpacket.hpp"
#include "xavframe.hpp"

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

void XDecodeTask::Do(XAVPacket &pkt) {

    if (m_stream_index_ != pkt.stream_index){
        return;
    }

    if (m_pkt_list_.Push(pkt)) {
        cout << GET_STR(P0) << flush;
    }

    Next(pkt);
}

void XDecodeTask::Main() {

    {
        unique_lock locker(m_mutex_);
        while (!m_is_exit_) {
            if ((m_frame_ = new_XAVFrame())) {
                break;
            }
            PRINT_ERR_TIPS(GET_STR(new_XAVFrame error!));
            locker.unlock();
            sleep_for(1ms);
            locker.lock();
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
                std::cout << GET_STR(D) << flush;
                m_need_view_ = true;
                if (m_frame_cache_) {
                    TRY_CATCH(CHECK_EXC(m_frames_.push_back(std::move(new_XAVFrame(*m_frame_)))));
                }
            }
        }
        sleep_for(1ms);
    }
}

bool XDecodeTask::Open(const XCodecParameters &parm){

    const auto c{XDecode::Create(parm.Codec_id(),false)};

    if (!c) {
        LOGERROR(GET_STR(Decode::Create failed!));
        return {};
    }

    parm.to_context(c);
    unique_lock locker(m_mutex_);
    m_decode_.set_codec_ctx(c);
    CHECK_FALSE_(m_decode_.Open(),return {});
    LOGDINFO(GET_STR(Open codec success!));
    return true;
}

bool XDecodeTask::Open(const XCodecParameters_sp &parm) {
    if (!parm){
        PRINT_ERR_TIPS(GET_STR(Parameters empty!));
        return {};
    }
#if 1
    return Open(*parm);
#else
    const auto c{XDecode::Create(parm->Codec_id(),false)};

    if (!c) {
        LOGERROR(GET_STR(Decode::Create failed!));
        return {};
    }

    parm->to_context(c);
    unique_lock locker(m_mutex_);
    m_decode_.set_codec_ctx(c);
    CHECK_FALSE_(m_decode_.Open(),return {});
    LOGDINFO(GET_STR(Open codec success!));
    return true;
#endif
}

XAVFrame_sp XDecodeTask::CopyFrame() {
    unique_lock locker(m_mutex_);

    if (m_frame_cache_) {
        if (m_frames_.empty()) {
            return {};
        }
        const auto frame{m_frames_.front()};
        m_frames_.pop_front();
        return frame;
    }

    if (!m_need_view_ || !m_frame_ || !m_frame_->buf[0]) {
        return {};
    }
    auto f{new_XAVFrame(*m_frame_)};
    m_need_view_ = false;
    return f;
}

XDecodeTask::~XDecodeTask() {
    XThread::Stop();
}
