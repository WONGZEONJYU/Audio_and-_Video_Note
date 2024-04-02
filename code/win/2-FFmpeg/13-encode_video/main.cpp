extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
}

#include <iostream>
#include <fstream>
#include <memory_resource>

template<typename F>
struct Destroyer final{
    Destroyer(const Destroyer&) = delete;
    Destroyer& operator=(const Destroyer&) = delete;
    explicit Destroyer(F &&f):fn(std::move(f)){}
    ~Destroyer() {
        fn();
    }
private:
    const F fn;
};

static std::string av_get_err(const int& errnum)
{
    constexpr auto ERROR_STRING_SIZE {1024};
    char err_buf[ERROR_STRING_SIZE]{};
    av_strerror(errnum, err_buf, std::size(err_buf));
    return {err_buf};
}

static int encode(AVCodecContext &context,const AVFrame &frame,AVPacket &packet,std::ofstream &out_file){

    if (frame.extended_data){
        std::cout << "Send frame " << frame.pts << "\n";
    }

    auto ret {frame.extended_data ? avcodec_send_frame( &context,&frame) :avcodec_send_frame( &context, nullptr)};
    if (ret < 0){
        std::cerr << "avcodec_send_frame failed : " << ret << " " << av_get_err(ret) << "\n";
        return -1;
    }

    for (;;) {
        ret = avcodec_receive_packet(&context,&packet);
        if (AVERROR_EOF == ret || AVERROR(EAGAIN) == ret){
            return 0;
        }else if(ret < 0){
            std::cerr << "avcodec_receive_packet failed : " << ret << " " << av_get_err(ret) << "\n";
            return ret;
        }else{}

        if (packet.flags & AV_PKT_FLAG_KEY){
            std::cout << "Write packet flags: " << packet.flags << ", pts: " << packet.pts << ", dts: " << packet.dts << ", size: " << packet.size << "\n";
        }

        if (!packet.flags){
            std::cout << "Write packet flags: " << packet.flags << ", pts: " << packet.pts << ", dts: " << packet.dts << ", size: " << packet.size << "\n";
        }

        out_file.write(reinterpret_cast<const char* >(packet.data),packet.size);
    }
    return 0;
}

int main(const int argc,const char *argv[]) {

    if (argc < 4){
        std::cerr << "Usage: " << argv[0] << "< input_file out_file codec_name >, argc:%d\n";
        return -1;
    }

    const auto encode_name{argv[3]};
    std::pmr::unsynchronized_pool_resource mptool;
    uint8_t *one_frame_buf{};
    int one_frame_size{};
    std::ifstream in_yuv_file(argv[1],std::ios::binary);
    std::ofstream out_h264_file(argv[2],std::ios::binary);
    const AVCodec *codec{};
    AVCodecContext *avCodecContext{};
    AVFrame *frame{};
    AVPacket *packet{};

    const Destroyer d([&](){
        if (one_frame_size){
            mptool.deallocate(one_frame_buf,one_frame_size);
        }
        out_h264_file.close();
        in_yuv_file.close();
        avcodec_free_context(&avCodecContext);
        av_frame_free(&frame);
        av_packet_free(&packet);
    });

    if(!in_yuv_file){
        std::cerr << "in_yuv_file open failed\n";
        return -1;
    }

    if (!out_h264_file){
        std::cerr << "open out_h264_file failed\n";
        return -1;
    }

    /* 查找指定的编码器 */
    codec = avcodec_find_encoder_by_name(encode_name);
    if (!codec) {
        std::cerr << "Codec "<< encode_name << " not found\n";
        return -1;
    }

    avCodecContext = avcodec_alloc_context3(codec);
    if (!avCodecContext){
        std::cerr << "avcodec_alloc_context3 failed\n";
        return -1;
    }

    /* 设置分辨率*/
    avCodecContext->width = 1280;
    avCodecContext->height = 720;
    /* 设置time base */
    avCodecContext->time_base = {1,25};
    avCodecContext->framerate = {25,1};

    /* 设置I帧间隔
     如果frame->pict_type设置为AV_PICTURE_TYPE_I,则忽略gop_size的设置,一直当做I帧进行编码
    */
    avCodecContext->gop_size = 25;/*I帧间隔*/
    //avCodecContext->max_b_frames = 1;
    avCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;

    if (AV_CODEC_ID_H264 == codec->id){

        if (av_opt_set(avCodecContext->priv_data, "preset", "medium", 0) < 0){
            std::cerr << "av_opt_set preset failed\n";
        }

        if (av_opt_set(avCodecContext->priv_data, "profile", "main", 0) < 0){
            std::cerr << "av_opt_set profile failed\n";
        }

        if (av_opt_set(avCodecContext->priv_data, "tune","zerolatency",0) < 0){
            std::cerr << "av_opt_set tune failed\n";
        }

    }

    /*设置编码器参数*/

    /* 设置bitrate */
    avCodecContext->bit_rate = 3000000;

//    avCodecContext->rc_max_rate = 3000000;
//    avCodecContext->rc_min_rate = 3000000;
//    avCodecContext->rc_buffer_size = 2000000;
//    avCodecContext->thread_count = 4;  //开了多线程后也会导致帧输出延迟,需要缓存thread_count帧后再编程。
//    avCodecContext->thread_type = FF_THREAD_FRAME; // 并设置为FF_THREAD_FRAME

    /* 对于H264 AV_CODEC_FLAG_GLOBAL_HEADER  设置则只包含I帧，此时sps pps需要从avCodecContext->extradata读取
     *  不设置则每个I帧都带 sps pps sei
     */

//    avCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER; // 存本地文件时不要去设置
    auto ret{avcodec_open2(avCodecContext,codec, nullptr)};

    if (ret < 0){
        std::cerr << "avcodec_open2 failed : " << ret << " " << av_get_err(ret) << "\n";
        return -1;
    }

    std::cout << "thread_count: " << avCodecContext->thread_count <<
                ", thread_type: " << avCodecContext->thread_type << "\n";

    packet = av_packet_alloc();
    if (!packet){
        std::cerr << "av_packet_alloc failed\n";
        return -1;
    }

    frame = av_frame_alloc();
    if (!frame){
        std::cerr << "av_frame_alloc failed\n";
        return -1;
    }

    // 为frame分配buffer
    frame->format = avCodecContext->pix_fmt;
    frame->width  = avCodecContext->width;
    frame->height = avCodecContext->height;
    ret = av_frame_get_buffer(frame,0);
    if (ret < 0){
        std::cerr << "Could not allocate the video frame data " << av_get_err(ret) << "\n";
        return -1;
    }
    
    return 0;
}
