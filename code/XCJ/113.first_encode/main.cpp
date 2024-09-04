extern "C"{
#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
}

#include <iostream>
#include <thread>
#include "XAVFrame.hpp"
#include "XAVPacket.hpp"
#include "XHelper.hpp"

int main(const int argc,const char *argv[]) {

    XAVFrame_sptr frame;
    XAVPacket_sptr packet;
    AVCodecContext *codec_ctx{};

    const Destroyer d([&]{
        avcodec_free_context(&codec_ctx);
        frame.reset();
        packet.reset();
    });

    auto codec{avcodec_find_encoder(AV_CODEC_ID_HEVC)};
    //auto codec{avcodec_find_encoder(AV_CODEC_ID_H264)};

    if (!codec) {
        std::cerr << "codec not found!\n";
        return -1;
    }

    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        std::cerr << "codec_ctx is empty\n";
        return -1;
    }

    codec_ctx->width = 400;
    codec_ctx->height = 300;
    codec_ctx->time_base = {1,25};
    codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    codec_ctx->thread_count = static_cast<int>(std::thread::hardware_concurrency());

    FF_ERR_OUT(avcodec_open2(codec_ctx,codec,{}),return -1);

    TRY_CATCH(CHECK_EXC(frame = new_XAVFrame()),return -1);

    frame->width = codec_ctx->width;
    frame->height = codec_ctx->height;
    frame->format = codec_ctx->pix_fmt;
    FF_ERR_OUT(frame->Get_Buffer(0),return -1);

    TRY_CATCH(CHECK_EXC(packet = new_XAVPacket()),return -1);
    //生成YUV数据
    for(int i{};i < 250 ;++i) {

        while (true){
            const auto ret{avcodec_receive_packet(codec_ctx, packet.get())};
            if (AVERROR_EOF == ret){
                FF_ERR_OUT(ret);
                break;
            } else if (AVERROR(EAGAIN) == ret){
                break;
            } else if (ret < 0){
                FF_ERR_OUT(ret);
                return -1;
            } else{
                std::cerr << packet.get()->size << "\n";
            }
        }

        //Y
        for (int y{}; y < codec_ctx->height; ++y) {
            for (int x{}; x < codec_ctx->width; ++x) {
                frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
            }
        }
        //UV
        for (int y {}; y < codec_ctx->height / 2; ++y) {
            for (int x {}; x < codec_ctx->width / 2; ++x) {
                frame->data[1][ y * frame->linesize[1] + x] = 128 + y + i * 2;
                frame->data[2][ y * frame->linesize[2] + x] = 64 + y + i * 5;
            }
        }

        frame->pts = i;
        const auto ret{avcodec_send_frame(codec_ctx,frame.get())};
        if (AVERROR_EOF == ret) {
            FF_ERR_OUT(ret);
            break;
        }else if (AVERROR(EAGAIN) == ret) {
            continue;
        } else if(ret < 0){
            FF_ERR_OUT(ret);
            return -1;
        } else{}
    }

    std::cerr << "\nencode success!\n\n";
    return 0;
}
