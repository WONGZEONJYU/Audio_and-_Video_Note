#include "xcamera_record.hpp"
#include <xdemuxtask.hpp>
#include <xmuxtask.hpp>

void XCameraRecorde::Main() {

    XDemuxTask demux_task;
    XMuxTask mux_task;

    CHECK_FALSE_(!m_rtsp_url_.empty(),
        PRINT_ERR_TIPS(GET_STR(rtsp url is empty!));return);

    //自动重连
    while (!m_is_exit_) {
        if (demux_task.Open(m_rtsp_url_)) { //最坏情况阻塞一秒
            break;
        }
        XHelper::MSleep(10);
    }

    //获取到音视频参数
    const auto vp{demux_task.CopyVideoParm()};
    CHECK_FALSE_(vp.operator bool(),demux_task.Stop();return);

    demux_task.Start(); //启动解封装,提前启动,防止查超时

    const auto ap{demux_task.CopyAudioParm()};

    CHECK_FALSE_(mux_task.Open(m_rtsp_url_,*vp,ap ? *ap : XCodecParameters()),
        demux_task.Stop();mux_task.Stop();return);

    demux_task.set_next(&mux_task);
    mux_task.Start();
}
