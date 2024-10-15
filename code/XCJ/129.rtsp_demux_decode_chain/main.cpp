#include <iostream>
#include "xtools.hpp"
#include "xdemuxtask.hpp"
#include "xmuxtask.h"

//rtmp://liteavapp.qcloud.com/live/liteavdemoplayerstreamid

static inline constexpr auto CAM01{"rtsp://admin:123456@192.168.50.123/stream0"};
static inline constexpr auto CAM02{"rtsp://admin:123456@10.10.10.254/stream0"};

int main(const int argc,const char *argv[]) {

    XDemuxTask demuxTask;

    while (true){
        if (demuxTask.Open(CAM01)){
            break;
        }
        XHelper::MSleep(100);
    }

    auto vparm{demuxTask.CopyVideoParm()};
    auto aparm{demuxTask.CopyAudioParm()};

    XMuxTask muxTask;
    muxTask.Open("rtsp_out1.mp4",vparm,aparm);
    demuxTask.set_next(&muxTask);
    demuxTask.Start();

    muxTask.Start();
    XHelper::MSleep(5000);
    muxTask.Stop();

    muxTask.Open("rtsp_out2.mp4",vparm,aparm);
    muxTask.Start();
    XHelper::MSleep(2*5000);
    muxTask.Stop();

    getchar();
    return 0;
}
