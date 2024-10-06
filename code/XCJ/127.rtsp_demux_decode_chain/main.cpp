#include <iostream>
#include "xtools.hpp"
#include "xdemuxtask.hpp"

//rtmp://liteavapp.qcloud.com/live/liteavdemoplayerstreamid

#define CAM1 \
"rtsp://admin:123456@10.0.0.123/stream0"

int main(const int argc,const char *argv[]) {

    XDemuxTask det;

    while (true){
        if (det.Open(CAM1)){
            break;
        }
        XHelper::MSleep(100);
    }

    det.Start();
    getchar();
    return 0;
}
