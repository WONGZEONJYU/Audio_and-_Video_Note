#include "xdecodetask.hpp"
#include "xcodec_parameters.hpp"
#include "xdecode.hpp"
#include "xavpacket.hpp"
#include "xavframe.hpp"
#include <sstream>

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

void XDecodeTask::Do(XAVPacket &pkt) {

    if (m_is_exit_) {
        return;
    }

    if (m_stream_index_ != pkt.stream_index){
        return;
    }

    if (m_pkt_list_.Push(pkt)) {
        // cout << " " << GET_STR(demux_index:) << " "
        // << pkt.stream_index << " " << flush;
    }else {
        stringstream ss;
        ss << GET_STR(stream_index:) << " " << pkt.stream_index << " " << GET_STR(push error!) << " ";
        LOG_ERROR(ss.str());
    }

    while (m_block_size> 0 && !m_is_exit_) {
        if (m_pkt_list_.Size() > m_block_size) {
            sleep_for(1ms);
            continue;
        }
        break;
    }

    Next(pkt);
}

void XDecodeTask::Main() {

    if (!m_is_open_) {
        return;
    }

    {
        unique_lock locker(m_mutex_);
        while (!m_is_exit_) {
            if ((m_frame_ = new_XAVFrame())) {
                break;
            }
            LOG_ERROR(GET_STR(retry new_XAVFrame!));
            locker.unlock();
            MSleep(1);
            locker.lock();
        }
    }

    while (!m_is_exit_){

        if (is_pause()){
            MSleep(1);
            continue;
        }

        int64_t curr_pts{-1};

        {
            unique_lock locker(m_mutex_);
            if (m_decode_.Receive(*m_frame_)){
                //cout << " " << GET_STR(Decode) << " " << av_get_media_type_string(media_type) << " " << flush;
                m_need_view_ = true;
                curr_pts = m_frame_->pts; //获取解码后的pts
                m_curr_ms_ = XHelper::XRescale(m_frame_->pts,m_paras_->time_base(),{1,1000});
                if (m_frame_cache_) {
                    TRY_CATCH(CHECK_EXC(m_frames_.emplace_back(new_XAVFrame(*m_frame_))));
                }
            }
        }

        while (!m_is_exit_) { //同步
            if (m_sync_pts_ >= 0 && curr_pts > m_sync_pts_) {
                MSleep(1);
                continue;
            }
            break;
        }

        const auto pkt{m_pkt_list_.Pop()};
        if (!pkt) {
            MSleep(1);
            continue;
        }

        if (!m_decode_.Send(*pkt)) {
            MSleep(1);
            continue;
        }

        MSleep(1);
    }
}

bool XDecodeTask::Open(const XCodecParameters &parm){

    m_is_open_ = false;
    AVCodecContext * c{};
    IS_NULLPTR(c = XDecode::Create(parm.Codec_id(),false),
        LOG_ERROR(GET_STR(Decode::Create failed!));return {};);

    parm.to_context(c);
    unique_lock locker(m_mutex_);
    m_decode_.set_codec_ctx(c);
    CHECK_FALSE_(m_decode_.Open(),return {});
    LOG_INFO(GET_STR(Open codec success!));
    m_is_open_ = true;
    return m_is_open_;
}

bool XDecodeTask::Open(const XCodecParameters_sp &parm) {
    IS_SMART_NULLPTR(parm, return {});
    m_paras_ = parm;
    return Open(*parm);
}

void XDecodeTask::Stop() {
    XThread::Stop();
    m_is_open_ = false;
    m_stream_index_ = -1;
    m_sync_pts_ = -1;
    m_block_size = -1;
    m_pkt_list_.Clear();
    unique_lock locker(m_mutex_);
    m_frames_.clear();
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

    m_need_view_ = false;
    return new_XAVFrame(*m_frame_);
}

XDecodeTask::~XDecodeTask() {
    cerr << "begin " << __FUNCTION__ << " current thread_id = "
        << XHelper::present_thread_id() << "\n";
    Stop();
    cerr << "end " << __FUNCTION__ << " current thread_id = "
        << XHelper::present_thread_id() << "\n";
}
