#include "xplay.hpp"
#include "xvideo_view.hpp"
#include "xaudio_play.hpp"

bool XPlay::Open(const std::string &url, void *win_id) {

    CHECK_FALSE_(!url.empty(), PRINT_ERR_TIPS(GET_STR(url is empty));return {});
    CHECK_FALSE_(m_demuxTask_.Open(url),return {});

    const auto vp{m_demuxTask_.CopyVideoParm()},
            ap{m_demuxTask_.CopyAudioParm()};

    if (vp) {
        CHECK_FALSE_(m_video_decode_task_.Open(vp),return {});
        m_video_decode_task_.set_stream_index(m_demuxTask_.video_index());
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
        CHECK_FALSE_(xAudio()->Open(ap),return {});
    }else {
        m_demuxTask_.set_sync_type(SYNC_VIDEO);
    }

    m_demuxTask_.set_next(this); //解封装后数据传给当前对象
    return true;
}

void XPlay::Start() {

    m_demuxTask_.Start();
    if (m_video_decode_task_) {
        m_video_decode_task_.Start();
    }
    if (m_audio_decode_task_) {
        m_audio_decode_task_.Start();
    }
    XThread::Start();
}

void XPlay::Main() {

    while (!m_is_exit_) {

    }
}

void XPlay::Update() {

    if (m_videoView_) {
        if (const auto f{m_video_decode_task_.CopyFrame()}) {
            m_videoView_->DrawFrame(*f);
        }
    }

    if (const auto f{m_audio_decode_task_.CopyFrame()}) {
        xAudio()->Push(*f);
    }
}

void XPlay::Do(XAVPacket &xav_packet) {
    if (m_video_decode_task_) {
        m_video_decode_task_.Do(xav_packet);
    }
    if (m_audio_decode_task_) {
        m_audio_decode_task_.Do(xav_packet);
    }
}

XPlay::~XPlay() {
    XThread::Stop();
}
