
#ifndef AVHELPER_H
#define AVHELPER_H

#include <iostream>
#include <string>

extern "C"{
#include <libavutil/error.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>
}

namespace AVHelper {

    std::string av_get_err(const int&);

    template<typename F,typename ...Args>
    void decode(AVCodecContext *codec_ctx,const AVPacket *pkt,AVFrame *frame,F&& f,Args&& ...args) noexcept(false)
    {
        /* send the packet with the compressed data to the decoder */
        auto ret { avcodec_send_packet(codec_ctx, pkt) };

        if(AVERROR(EAGAIN) == ret || AVERROR_EOF == ret){
            std::cerr << "Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n";
        }else if (ret < 0){
            const std::string errmsg ( "Error submitting the packet to the decoder, err: " + av_get_err(ret) +
                      " , pkt_size : " + std::to_string(pkt->size) + "\n");

            throw std::runtime_error(errmsg);
        }else{}

        /* read all the output frames (infile general there may be any number of them */
        for(;;){
            // 对于frame, avcodec_receive_frame内部每次都先调用
            ret = avcodec_receive_frame(codec_ctx, frame);
            if (AVERROR(EAGAIN) == ret  || AVERROR_EOF == ret) {
                std::cerr << "avcodec_receive_frame failed: " << av_get_err(ret) << "\n";
                return;
            }else if (ret < 0){
                throw std::runtime_error("Error during decoding : " + av_get_err(ret) + "\n");
            }

            f(args...);
        }
    }

    template<typename F,typename ...Args>
    void encode(AVCodecContext *codec_ctx,const AVFrame *frame,AVPacket *packet,F&& f,Args&& ...args) noexcept(false)
    {
        auto ret{avcodec_send_frame(codec_ctx,frame)};

        if (AVERROR(EAGAIN) == ret || AVERROR_EOF == ret){
            std::cerr << "Receive_packet and send_frame both returned EAGAIN, which is an API violation.\n";
        }else if (ret < 0) {
            throw std::runtime_error("avcodec_send_frame failed : " +  std::to_string(ret) + "\t" + av_get_err(ret) + "\n");

        }else{}

        for (;;) {
            ret = avcodec_receive_packet(codec_ctx,packet);
            if (AVERROR_EOF == ret || AVERROR(EAGAIN) == ret){
                //std::cerr << "avcodec_receive_packet failed: " << ret << "\t" << av_get_err(ret) << "\n";
                return;
            }else if(ret < 0){
                throw std::runtime_error("Error during encoding: " + std::to_string(ret) + "\t" + av_get_err(ret) + "\n");
            }else{}

            f(args...);

        }
    }

    void log_packet(const AVFormatContext &, const AVPacket &);
}

#endif
