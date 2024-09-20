extern "C"{
#include <libavformat/avformat.h>
}
#include <iostream>
#include <thread>
#include "xhelper.hpp"
#include "xavpacket.hpp"

using namespace std::chrono;
using namespace std::this_thread;

int main(const int argc,const char *argv[]) {

    AVFormatContext *ic{};
    const Destroyer d([&]{
        avformat_close_input(&ic);
    });

    constexpr auto url{"v1080.mp4"};
    FF_ERR_OUT(avformat_open_input(&ic,url, nullptr, nullptr),return -1);

    FF_ERR_OUT(avformat_find_stream_info(ic, nullptr),return -1);

    av_dump_format(ic,0, url,0);

    AVStream *vs{},*as{};

    for (int i {}; i < ic->nb_streams; ++i) {
        const auto codecpar{ic->streams[i]->codecpar};
        if (AVMEDIA_TYPE_VIDEO == codecpar->codec_type){
            vs = ic->streams[i];
            std::cerr << "width: " << codecpar->width << " height: " << codecpar->height << "\n";
        } else if (AVMEDIA_TYPE_AUDIO == codecpar->codec_type){
            std::cerr << "sample_rate: " << codecpar->sample_rate << "\n";
            as = ic->streams[i];
        } else{}
    }

    XAVPacket packet;
    while (true){
        int re;
        FF_ERR_OUT(re = av_read_frame(ic, &packet));
        if (re < 0) {
            break;
        }

        if (vs && packet.stream_index == vs->index){
            std::cerr << "video: packet.size: " << packet.size << "\t" <<
                      "packet.dts: " << packet.dts << "\t" <<
                      "packet.pts: " << packet.pts << "\n";
        } else if (as && packet.stream_index == as->index){
            std::cerr << "audio: packet.size: " << packet.size << "\t" <<
                      "packet.dts: " << packet.dts << "\t" <<
                      "packet.pts: " << packet.pts << "\n";
        } else{}

        packet.Reset();
        sleep_for(1ms);
    }

    return 0;
}
