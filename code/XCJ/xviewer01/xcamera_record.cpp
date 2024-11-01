#include "xcamera_record.hpp"
#include <xdemuxtask.hpp>
#include <xmuxtask.hpp>
#include <chrono>
#include <iomanip>

using namespace std;
using namespace std::chrono;

static string GetFileName(const string &path) {
    const auto t{system_clock::to_time_t(system_clock::now())};
    const auto time_str{put_time(localtime(&t), GET_STR(%Y_%m_%d_%H_%M_%S))};
    stringstream ss;
    ss << path << GET_STR(/) << GET_STR(cam_) << time_str << GET_STR(.mp4);
    return ss.str();
}

void XCameraRecord::Main() {

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

    CHECK_FALSE_(mux_task.Open(GetFileName(m_save_path_),vp.get(),ap.get()),
        demux_task.Stop();mux_task.Stop();return);

    demux_task.set_next(&mux_task);
    mux_task.Start();

    auto present_time{XHelper::Get_time_ms()};
    while (!m_is_exit_) {

        if (const auto now{XHelper::Get_time_ms()};now - present_time > m_file_sec_ * 1000) {
            present_time = now;
            mux_task.Stop();
            CHECK_FALSE_(mux_task.Open(GetFileName(m_save_path_),vp.get(),ap.get()),
                demux_task.Stop();mux_task.Stop();return);
            mux_task.Start();
        }

        XHelper::MSleep(10);
    }

    mux_task.Stop();
    demux_task.Stop();
}
