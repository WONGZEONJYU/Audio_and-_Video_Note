
#ifndef AVHELPER_H
#define AVHELPER_H

extern "C"{
#include <libavutil/error.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>
}

#include <iostream>
#include <string>

namespace AVHelper {

    std::string av_get_err(const int& errnum);

    template<typename F,typename ...Args>
    bool decode(AVCodecContext &dec_ctx,const AVPacket &pkt,AVFrame &frame,F&& f,Args&& ...args) noexcept(false)
    {
        /* send the packet with the compressed data to the decoder */

        auto ret { pkt.data ? avcodec_send_packet(&dec_ctx, &pkt) : avcodec_send_packet(&dec_ctx, nullptr)};

        if(AVERROR(EAGAIN) == ret || AVERROR_EOF == ret){
            std::cerr << "Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n";
        }else if (ret < 0){
            std::cerr << "Error submitting the packet to the decoder, err: " << av_get_err(ret) <<
                      " , pkt_size : " << pkt.size << "\n";
            return false;
        }else{}

        /* read all the output frames (infile general there may be any number of them */
        for(;;){
            // 对于frame, avcodec_receive_frame内部每次都先调用
            ret = avcodec_receive_frame(&dec_ctx, &frame);
            if (AVERROR(EAGAIN) == ret  || AVERROR_EOF == ret ) {
                std::cerr << "avcodec_receive_frame failed: " << av_get_err(ret) << "\n";
                return true;
            }else if (ret < 0){
                std::cerr << "Error during decoding : " << av_get_err(ret) << "\n";
                return false;
            }

            f(args...);
        }
    }

    template<typename F,typename ...Args>
    bool encode(AVCodecContext &context,const AVFrame &frame,AVPacket &packet,F&& f,Args&& ...args) noexcept(false)
    {
        auto ret {frame.data[0] ? avcodec_send_frame( &context,&frame) : avcodec_send_frame(&context, nullptr)};

        if (AVERROR(EAGAIN) == ret || AVERROR_EOF == ret ){
            std::cerr << "Receive_packet and send_frame both returned EAGAIN, which is an API violation.\n";
        }else if (ret < 0) {
            std::cerr << "avcodec_send_frame failed : " << ret << "\t" << av_get_err(ret) << "\n";
            return false;
        }else{}

        for (;;) {
            ret = avcodec_receive_packet(&context,&packet);
            if (AVERROR_EOF == ret || AVERROR(EAGAIN) == ret){
                std::cerr << "avcodec_receive_packet failed: " << ret << "\t" << av_get_err(ret) << "\n";
                return true;
            }else if(ret < 0){
                std::cerr << "Error during encoding: " << ret << "\t" << av_get_err(ret) << "\n";
                return false;
            }else{}
#if 0
            if (packet.flags & AV_PKT_FLAG_KEY){
                std::cout << "Write packet flags: " << packet.flags << ", pts: " << packet.pts << ", dts: " << packet.dts << ", size: " << packet.size << "\n";
            }

            if (!packet.flags){
                std::cout << "Write packet flags: " << packet.flags << ", pts: " << packet.pts << ", dts: " << packet.dts << ", size: " << packet.size << "\n";
            }
#endif
            f(args...);
        }
    }

    void log_packet(const AVFormatContext &fmt_ctx, const AVPacket &pkt);
}


#endif //AVHELPER_H
