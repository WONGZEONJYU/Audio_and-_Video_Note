#include <iostream>
#include "xtools.hpp"
#include "xdemuxtask.hpp"
#include "xdecodetask.h"
#include "xvideo_view.hpp"

//rtmp://liteavapp.qcloud.com/live/liteavdemoplayerstreamid

static inline constexpr auto CAM01{"rtsp://admin:123456@192.168.50.123/stream0"};
static inline constexpr auto CAM02 {"rtsp://admin:123456@10.10.10.254/stream0"};

int main(const int argc,const char *argv[]) {

    XDemuxTask demuxTask;

    while (true){
        if (demuxTask.Open(CAM01)){
            break;
        }
        XHelper::MSleep(100);
    }

    auto parm{demuxTask.CopyVideoParm()};



    getchar();
    return 0;
}