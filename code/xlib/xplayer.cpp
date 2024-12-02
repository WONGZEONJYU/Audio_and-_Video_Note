#include "xplayer.hpp"
#include "xvideo_view.hpp"
#include "xaudio_play.hpp"
#include "xavpacket.hpp"

using namespace std;

bool XPlayer::open_helper(const std::string &url){





}

bool XPlayer::Open(const string &url,void * const win_id) {

    CHECK_FALSE_(!url.empty(),PRINT_ERR_TIPS(GET_STR(url is empty));return {});
    Stop();
    m_is_open_ = false;
    CHECK_FALSE_(m_demuxTask_.Open(url),return {});

    const auto vp{m_demuxTask_.CopyVideoParm()},
            ap{m_demuxTask_.CopyAudioParm()};

    if (vp) {
        m_video_params_ = vp;
        m_total_ms_ = vp->total_ms();
        CHECK_FALSE_(m_video_decode_task_.Open(vp),return {});
        m_video_decode_task_.set_stream_index(m_demuxTask_.video_index());
        m_video_decode_task_.set_block_size(100);
        if (!m_ex_func_ && !m_videoView_) {
            IS_SMART_NULLPTR(m_videoView_ = XVideoView::create_sp(),return {});
            m_videoView_->Set_Win_ID(win_id);
            CHECK_FALSE_(m_videoView_->Init(*vp),return {});
        }
    }

    if (ap) {
        CHECK_FALSE_(m_audio_decode_task_.Open(ap),return {});
        m_audio_decode_task_.set_stream_index(m_demuxTask_.audio_index());
        m_audio_decode_task_.set_frame_cache(true);
        m_audio_decode_task_.set_block_size(100);
        CHECK_FALSE_(xAudio()->Open(ap),return {});
    }else {
        m_demuxTask_.set_sync_type(SYNC_VIDEO);
    }

    m_demuxTask_.set_next(this); //解封装后数据传给当前对象
    m_is_open_ = true;
    return true;
}

void XPlayer::Stop() {
    XThread::Stop();
    m_demuxTask_.Stop();
    m_video_decode_task_.Stop();
    m_audio_decode_task_.Stop();
    Wait();
    m_demuxTask_.Wait();
    m_video_decode_task_.Wait();
    m_audio_decode_task_.Wait();
    m_is_open_ = false;
    m_videoView_.reset();
    m_ex_func_ = {};
    xAudio()->Close();
}

void XPlayer::Start() {

    if (!m_is_open_){
        return;
    }

    if (is_exit()){
        XThread::Start();
    }

    if (m_demuxTask_.is_exit()){
        m_demuxTask_.Start();
    }

    if (m_video_decode_task_ && m_video_decode_task_.is_exit()) {
        m_video_decode_task_.Start();
    }

    if (m_audio_decode_task_ && m_audio_decode_task_.is_exit()) {
        m_audio_decode_task_.Start();
    }
}

void XPlayer::Main() {

    const auto vp{m_demuxTask_.CopyVideoParm()},
                ap{m_demuxTask_.CopyAudioParm()};

    while (!is_exit()) {

        if (is_pause()){
            MSleep(1);
            continue;
        }

        m_pos_ms_ = m_video_decode_task_.curr_pts_ms();

        if (ap) {
            const auto sync{XHelper::XRescale(xAudio()->curr_pts(),
                         ap->x_time_base(),
                         vp->x_time_base())};
            //cerr << __FUNCTION__ << " sync = " << sync << "\n";
            m_video_decode_task_.set_sync_pts(sync);
            m_audio_decode_task_.set_sync_pts(xAudio()->curr_pts() + 10000);
        }

        MSleep(1);
    }
}

bool XPlayer::win_is_exit(){
    return m_videoView_ && m_videoView_->Is_Exit_Window();
}

void XPlayer::pause(const bool &b){
    XThread::pause(b);
    m_demuxTask_.pause(b);
    m_audio_decode_task_.pause(b);
    m_video_decode_task_.pause(b);
    xAudio()->Pause(b);
}

bool XPlayer::Seek(const int64_t &ms){

    const auto state{is_pause()};
    pause(true);
    m_demuxTask_.Clear();
    m_video_decode_task_.Clear();
    m_audio_decode_task_.Clear();
    xAudio()->Clear();

    /**
     * 此处应该先暂停,再清理,可以有效解决在seek过程中可能出现花屏问题
     * 先暂停好处是,所有的线程马上暂停,再进行清理动作可以避免线程带来的线程竞争问题
     */

    if (!m_demuxTask_.Seek(ms)) {
        return {};
    }

    bool b{};
    while (!is_exit()) {
        XAVPacket pkt;
        if (!m_demuxTask_.ReadVideoPacket(pkt)) {
            break;
        }

        if (m_video_decode_task_.Decode(pkt) &&
            m_video_decode_task_.curr_pts_ms() >= ms) {
            Update();
            m_pos_ms_ = m_video_decode_task_.curr_pts_ms();
            b = true;
            break;
        }
    }

    pause(state);
    return b;
}

XCodecParameters_sp XPlayer::get_video_params() const {
    return m_demuxTask_.CopyVideoParm();
}

void XPlayer::SetSpeed(const float &speed) {
    if (m_demuxTask_.audio_index() >= 0) {
        xAudio()->set_speed(speed);
    }else {
        m_demuxTask_.set_speed(speed);
    }
}

[[maybe_unused]] void XPlayer::Update() {

    if (const auto af{m_audio_decode_task_.CopyFrame()}) {
        xAudio()->Push(*af);
    }

    if (const auto vf{m_video_decode_task_.CopyFrame()}) {
        if (m_videoView_){
            m_videoView_->DrawFrame(*vf);
        }
        unique_lock locker(m_mux_);
        if (m_ex_func_){
            m_ex_func_(*vf);
        }
    }
}

void XPlayer::Do(XAVPacket &pkt) {

    if (m_video_decode_task_) {
        m_video_decode_task_.Do(pkt);
    }

    if (m_audio_decode_task_) {
        m_audio_decode_task_.Do(pkt);
    }
}

XPlayer::~XPlayer() {
    cerr << "begin " << __FUNCTION__ << " current thread_id = " <<
            XHelper::present_thread_id() << "\n";
    XPlayer::Stop();
    cerr << "end " << __FUNCTION__ << " current thread_id = " <<
              XHelper::present_thread_id() << "\n";
}
