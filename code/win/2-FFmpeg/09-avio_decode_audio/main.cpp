#include <iostream>
#include <fstream>
#include <memory_resource>

extern "C"{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

template<typename F>
struct Destroyer final{
    Destroyer(const Destroyer&) = delete;
    Destroyer& operator=(const Destroyer&) = delete;
    explicit Destroyer(F &&f):fn(std::move(f)){}
    ~Destroyer() {
        std::cerr << __FUNCTION__ << "\n";
        fn();
    }
private:
    F fn;
};

static inline constexpr auto BUF_SIZE{20480};

static void print_sample_format(const AVFrame &frame)
{
    std::cout << "ar-samplerate: " << frame.sample_rate << "Hz\n";
    std::cout << "ac-channel: "<< frame.ch_layout.nb_channels << "\n";
    std::cout << "f-format: " << frame.format << " " << av_get_sample_fmt_name(static_cast<AVSampleFormat>(frame.format)) << "\n";
    // 格式需要注意,实际存储到本地文件时已经改成交错模式
}

static std::string av_get_err(const int& errnum)
{
    constexpr auto ERROR_STRING_SIZE {1024};
    char err_buf[ERROR_STRING_SIZE]{};
    av_strerror(errnum, err_buf, std::size(err_buf));
    return {err_buf};
}

static void decode(AVCodecContext &dec_ctx,const AVPacket &pkt, AVFrame &frame,
                   std::ofstream &outfile)
{
    /* send the packet with the compressed data to the decoder */
    auto ret {avcodec_send_packet(&dec_ctx, &pkt)};

    if(AVERROR(EAGAIN) == ret){
        std::cerr << "Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n";
    }else if (ret < 0){
        std::cout << "Error submitting the packet to the decoder, err: " << av_get_err(ret) << " , pkt_size: " << pkt.size << "\n";
        return;
    }

    /* read all the output frames (infile general there may be any number of them */
    while (ret >= 0) {
        // 对于frame, avcodec_receive_frame内部每次都先调用
        ret = avcodec_receive_frame(&dec_ctx, &frame);
        if (AVERROR(EAGAIN) == ret || ret == AVERROR_EOF){
            return;
        }else if (ret < 0){
            std::cerr << "Error during decoding\n";
            exit(ret);
        }else{

        }

        const auto data_size {av_get_bytes_per_sample(dec_ctx.sample_fmt)};
        if (data_size < 0){
            /* This should not occur, checking just for paranoia */
            std::cout << "Failed to calculate data size\n";
            exit(-1);
        }

        static bool s_print_format {s_print_format ? s_print_format : (print_sample_format(frame),true)};
        //s_print_format = s_print_format ? s_print_format : (print_sample_format(frame),true);

        /**
            P表示Planar(平面),其数据格式排列方式为:
            LLLLLLRRRRRRLLLLLLRRRRRRLLLLLLRRRRRRL...(每个LLLLLLRRRRRR为一个音频帧)
            而不带P的数据格式(即交错排列)排列方式为:
            LRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLRL...(每个LR为一个音频样本)
            播放范例:ffplay -ar 48000 -ac 2 -f f32le believe.pcm
          **/

        for (int i {}; i < frame.nb_samples; i++){
            for (int ch {}; ch < dec_ctx.ch_layout.nb_channels; ++ch){
                //交错的方式写入,大部分float的格式输出
                outfile.write(reinterpret_cast<const char*>(frame.data[ch] + data_size * i),data_size);
            }
        }
    }
}

int main(const int argc,const char* argv[])
{
    if (argc < 3){
        std::cerr << "usage app input.aac out.pcm\n";
        return -1;
    }

    std::ifstream in_file(argv[1],std::ios::binary);
    std::ofstream out_file(argv[2],std::ios::binary);

    AVFormatContext *ifmt_ctx{};
    AVIOContext *io_ctx{};
    AVPacket pkt{};
    AVFrame frame{};
    av_packet_unref(&pkt);
    av_frame_unref(&frame);

    std::pmr::unsynchronized_pool_resource mptool;
    uint8_t* iobuff{};

    try{
        iobuff = static_cast<uint8_t*>(mptool.allocate(BUF_SIZE));
    }catch (const std::exception &e){
        std::cerr << "allocate faild : " << e.what() << "\n";
        return -1;
    };

    auto rres{[&](){
        in_file.close();
        out_file.close();
        av_packet_unref(&pkt);
        av_frame_unref(&frame);
        avformat_close_input(&ifmt_ctx);
        mptool.deallocate(iobuff,BUF_SIZE);
        mptool.release();
    }};

    Destroyer d(std::move(rres));
    //io_ctx = avio_alloc_context(iobuff,BUF_SIZE,0,static_cast<void*>(&in_file),nullptr,nullptr,nullptr,);

    return 0;
}
