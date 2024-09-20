extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}
#include <iostream>
#include <thread>
#include "xdecode.hpp"
#include "xvideo_view.hpp"
#include "xavpacket.hpp"
#include "xavframe.hpp"

using namespace std::chrono;
using namespace std::this_thread;

int main(const int argc,const char *argv[]) {

    AVFormatContext *ic{};
    auto view{XVideoView::create()};
    const Destroyer d([&]{
        avformat_close_input(&ic);
        delete view;
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

    if (!vs && !as){
        return -1;
    }

    view->Init(vs->codecpar->width,vs->codecpar->height,XVideoView::YUV420P);
    const auto codec_id{vs->codecpar->codec_id};

    XDecode v_dec;
    auto v_dec_ctx{XCodec::Create(codec_id, false)};
    v_dec.set_codec_ctx(v_dec_ctx);
    FF_ERR_OUT(avcodec_parameters_to_context(v_dec_ctx,vs->codecpar),return -1);
    if (!v_dec.Open()){
        return -1;
    }

    auto frame{v_dec.CreateFrame()};
    auto packet{new_XAVPacket()};

    while (true){
        int re;
        FF_ERR_OUT(re = av_read_frame(ic, packet.get()));
        if (re < 0) {
            break;
        }

        if (packet->stream_index == vs->index){
//            std::cerr << "video: packet.size: " << packet.size << "\t" <<
//                      "packet.dts: " << packet.dts << "\t" <<
//                      "packet.pts: " << packet.pts << "\n";

            if (v_dec.Send(packet)){
                while (v_dec.Receive(frame)){
                    std::cerr << frame->pts << "\n";
                    view->DrawFrame(frame);
                    if (view->Is_Exit_Window()){
                        return 0;
                    }
                }
            }
        }
#if 0
        else if (as && packet->stream_index == as->index){
//            std::cerr << "audio: packet.size: " << packet.size << "\t" <<
//                      "packet.dts: " << packet.dts << "\t" <<
//                      "packet.pts: " << packet.pts << "\n";
        } else{}
#endif
        packet->Reset();
    }

    auto frames{v_dec.Flush()};
    for (auto &item:frames) {
        std::cerr << item->pts << "\n";
        view->DrawFrame(frame);
        if (view->Is_Exit_Window()){
            return 0;
        }
    }

    return 0;
}
