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
        if (demuxTask.Open(CAM02)){
            break;
        }
        XHelper::MSleep(100);
    }

    auto parm{demuxTask.CopyVideoParm()};
    auto view{XVideoView::create()};
    view->Init(*parm);

    XDecodeTask decodeTask;

    if (!decodeTask.Open(*parm)){
        LOGERROR(GET_STR(decodeTask.Open failed!));
    } else{
        demuxTask.set_next(&decodeTask);
        demuxTask.Start();
        decodeTask.Start();
    }

    while (true){

        auto f{decodeTask.CopyFrame()};
        if (!f) {
            XHelper::MSleep(1);
            continue;
        }

        view->DrawFrame(*f);

        if(view->Is_Exit_Window()){
            break;
        }
    }
    //getchar();
    delete view;
    return 0;
}
