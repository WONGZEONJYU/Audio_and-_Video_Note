#include "xplayer.hpp"
#include "xvideo_view.hpp"
#include "xaudio_play.hpp"

using namespace std;

bool XPlayer::Open(const std::string &url,void * const win_id,const bool &ex_) {

    CHECK_FALSE_(!url.empty(),PRINT_ERR_TIPS(GET_STR(url is empty));return {});
    m_is_open_ = false;
    CHECK_FALSE_(m_demuxTask_.Open(url),return {});

    const auto vp{m_demuxTask_.CopyVideoParm()},
            ap{m_demuxTask_.CopyAudioParm()};

    if (vp) {
        CHECK_FALSE_(m_video_decode_task_.Open(vp),return {});
        m_video_decode_task_.set_stream_index(m_demuxTask_.video_index());
        m_video_decode_task_.set_block_size(100);
        m_video_decode_task_.set_frame_cache(true);
        if (!ex_ && !m_videoView_) {
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
    xAudio()->Close();
}

void XPlayer::Start() {

    if (!m_is_open_){
        return;
    }
    m_demuxTask_.Start();
    if (m_video_decode_task_) {
        m_video_decode_task_.Start();
    }
    if (m_audio_decode_task_) {
        m_audio_decode_task_.Start();
    }
    XThread::Start();
}

void XPlayer::Main() {

    const auto vp{m_demuxTask_.CopyVideoParm()},
                ap{m_demuxTask_.CopyAudioParm()};

    if (!ap) { //没有音频,无需用音频同步
        return;
    }

    while (!m_is_exit_) {
        const auto sync{XRescale(xAudio()->curr_pts(),
                                 ap->x_time_base(),
                                 vp->x_time_base())};
        m_video_decode_task_.set_sync_pts(sync);
        m_audio_decode_task_.set_sync_pts(xAudio()->curr_pts());
//        m_video_decode_task_.set_sync_pts(m_audio_decode_task_.now_pts());
        MSleep(1);
    }
}

bool XPlayer::win_is_exit(){
    return m_videoView_ ? m_videoView_->Is_Exit_Window() : false;
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

void XPlayer::Update() {

    if (m_videoView_) {
        if (const auto vf{m_video_decode_task_.CopyFrame()}) {
            m_videoView_->DrawFrame(*vf);
        }
    }

    if (const auto af{m_audio_decode_task_.CopyFrame()}) {
        xAudio()->Push(*af);
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
