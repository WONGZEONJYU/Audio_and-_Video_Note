#include "xplayer.hpp"
#include "xvideo_view.hpp"
#include "xaudio_play.hpp"

bool XPlayer::Open(const std::string &url, void *win_id) {

    CHECK_FALSE_(!url.empty(), PRINT_ERR_TIPS(GET_STR(url is empty));return {});
    m_is_open_ = false;
    CHECK_FALSE_(m_demuxTask_.Open(url),return {});

    const auto vp{m_demuxTask_.CopyVideoParm()},
            ap{m_demuxTask_.CopyAudioParm()};

    if (vp) {
        CHECK_FALSE_(m_video_decode_task_.Open(vp),return {});
        m_video_decode_task_.set_stream_index(m_demuxTask_.video_index());
        m_video_decode_task_.set_block_size(100);
        if (!m_videoView_) {
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

    while (!m_is_exit_) {

        if (ap && vp){
            const auto sync{XRescale(xAudio()->curr_pts(),
                                     ap->x_time_base(),
                                     vp->x_time_base())};

            m_video_decode_task_.set_sync_pts(sync);
        }

        m_audio_decode_task_.set_sync_pts(xAudio()->curr_pts() + 10000);
        XHelper::MSleep(1);
    }

    m_video_decode_task_.set_sync_pts(-1);
    m_audio_decode_task_.set_sync_pts(-1);
    //防止因为同步而导致线程卡住
}

bool XPlayer::win_is_exit(){
    if (m_videoView_){
        return m_videoView_->Is_Exit_Window();
    }
    return {};
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

void XPlayer::Do(XAVPacket &xav_packet) {
    if (m_video_decode_task_) {
        m_video_decode_task_.Do(xav_packet);
    }
    if (m_audio_decode_task_) {
        m_audio_decode_task_.Do(xav_packet);
    }
}

XPlayer::~XPlayer() {
    std::cerr << "begin " << __FUNCTION__ << " current thread_id = " <<
            XHelper::present_thread_id() << "\n";
    m_demuxTask_.Stop(); //先停掉解封装可以
    m_video_decode_task_.Stop();
    m_audio_decode_task_.Stop();
    XThread::Stop();
    std::cerr << "end " << __FUNCTION__ << " current thread_id = " <<
              XHelper::present_thread_id() << "\n";
}
