#include <iostream>
#include "xtools.hpp"
#include "xdemuxtask.hpp"
#include "xdecodetask.h"

//rtmp://liteavapp.qcloud.com/live/liteavdemoplayerstreamid

#define CAM1 \
"rtsp://admin:123456@192.168.50.123/stream0"

int main(const int argc,const char *argv[]) {

    XDemuxTask demuxTask;

    while (true){
        if (demuxTask.Open(CAM1)){
            break;
        }
        XHelper::MSleep(100);
    }

    auto parm{demuxTask.CopyVideoParm()};
    XDecodeTask decodeTask;

    if (!decodeTask.Open(*parm)){
        LOGERROR(GET_STR(decodeTask.Open failed!));
    } else{
        demuxTask.set_next(&decodeTask);
        demuxTask.Start();
        decodeTask.Start();
    }

    getchar();
    return 0;
}
