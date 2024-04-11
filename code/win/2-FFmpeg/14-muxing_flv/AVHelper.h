
#ifndef AVHELPER_H
#define AVHELPER_H

extern "C"{
#include <libavutil/error.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>
}

#include <string>

namespace AVHelper {
    std::string av_get_err(const int& errnum);
    bool decode(AVCodecContext &,const AVPacket &,AVFrame &);
    bool encode(AVCodecContext &,const AVFrame &,AVPacket &);
    void log_packet(const AVFormatContext &fmt_ctx, const AVPacket &pkt);
}


#endif //AVHELPER_H
