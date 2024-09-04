extern "C"{
#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
}

#include <iostream>
#include <thread>
#include "XHelper.hpp"

int main(const int argc,const char *argv[]) {

    auto codec{avcodec_find_encoder(AV_CODEC_ID_HEVC)};

    //auto codec{avcodec_find_encoder(AV_CODEC_ID_H264)};

    if (!codec) {
        std::cerr << "codec not found!\n";
        return -1;
    }

    auto codec_ctx{avcodec_alloc_context3(codec)};
    if (!codec_ctx) {
        std::cerr << "codec_ctx is empty\n";
        return -1;
    }

    codec_ctx->width = 400;
    codec_ctx->height = 300;
    codec_ctx->time_base = {1,25};
    codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    codec_ctx->thread_count = static_cast<int>(std::thread::hardware_concurrency());

    int ret;
    FF_ERR_OUT(ret = avcodec_open2(codec_ctx,codec, nullptr),return -1);

    std::cerr << "avcodec_open2 success!\n";

    avcodec_free_context(&codec_ctx);

    return 0;
}
