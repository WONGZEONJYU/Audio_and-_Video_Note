#include <iostream>
#include <fstream>

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
        fn();
    }
private:
    F fn;
};

static inline constexpr auto BUF_SIZE{20480};

static void print_sample_format(const AVFrame &frame)
{
    std::cout << "ar-samplerate: " << frame.sample_rate << "Hz\n" <<
                "ac-channel: "<< frame.ch_layout.nb_channels << "\n" <<
                "f-format: " << frame.format << " " <<
                av_get_sample_fmt_name(static_cast<AVSampleFormat>(frame.format)) << "\n";
    // 格式需要注意,实际存储到本地文件时已经改成交错模式
}

static std::string av_get_err(const int& errnum)
{
    constexpr auto ERROR_STRING_SIZE {1024};
    char err_buf[ERROR_STRING_SIZE]{};
    av_strerror(errnum, err_buf, std::size(err_buf));
    return {err_buf};
}

static int read_packet(void *opaque, uint8_t *buf,const int buf_size)
{
    auto &in_file{*static_cast<std::ifstream*>(opaque)};

    in_file.read(reinterpret_cast<char *>(buf),buf_size);
    const auto read_size{in_file.gcount()};
    std::cout << "read_packet read_size : " << read_size << ", buf_size : " << buf_size << "\n";

    if(read_size <=0) {
        return AVERROR_EOF; // 数据读取完毕
    }

    return static_cast<int>(read_size);
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

        static bool s_print_format {};
        s_print_format = s_print_format ? s_print_format : (print_sample_format(frame),true);

        /**
            P表示Planar(平面),其数据格式排列方式为:
            LLLLLLRRRRRRLLLLLLRRRRRRLLLLLLRRRRRRL...(每个LLLLLLRRRRRR为一个音频帧)
            而不带P的数据格式(即交错排列)排列方式为:
            LRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLRL...(每个LR为一个音频样本)
            播放范例:ffplay -ar 48000 -ac 2 -f f32le believe.pcm
          **/
        //std::cout << "frame.nb_samples : " << frame.nb_samples << "\n";
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

    AVIOContext *avio_ctx{};
    AVFormatContext *format_ctx{};
    const AVCodec *codec{};
    AVCodecContext *codec_ctx{};

    auto iobuff{av_malloc(BUF_SIZE)};

     auto rres{[&](){
         if (codec_ctx){
             avcodec_free_context(&codec_ctx);
         }
         if (format_ctx){
             avformat_close_input(&format_ctx);
         }
         if (avio_ctx){
             av_freep(&avio_ctx->buffer);
             avio_context_free(&avio_ctx);
         }
         if (in_file){
             in_file.close();
         }
         if(out_file){
             out_file.close();
         }
     }};

     Destroyer d(std::move(rres));

    if (!in_file){
        std::cerr << "open in_file failed\n";
        return -1;
    }

    if (!out_file){
        std::cerr << "open out_file faild\n";
        return -1;
    }

    avio_ctx = avio_alloc_context(static_cast<uint8_t*>(iobuff),BUF_SIZE,0,static_cast<void*>(&in_file),
        read_packet,nullptr,nullptr);

    if (!avio_ctx) {
        std::cerr << "avio_alloc_context failed : \n";
        return -1;
    }

    format_ctx = avformat_alloc_context();

    if (!format_ctx) {
        std::cerr << "avformat_alloc_context failed\n";
        return -1;
    }

    format_ctx->pb = avio_ctx;

    auto ret {avformat_open_input(&format_ctx, nullptr, nullptr, nullptr)};
    if (ret < 0) {
        std::cerr << "avformat_open_input failed : " << av_get_err(ret) << "\n";
        return -1;
    }

    // 编码器查找
    codec = avcodec_find_decoder(AV_CODEC_ID_AAC);
    if(!codec) {
        std::cerr << "avcodec_find_decoder failed\n";
        return -1;
    }

    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        std::cerr << "avcodec_alloc_context3 failed\n";
        return -1;
    }

    ret = avcodec_open2(codec_ctx, codec, nullptr);
    if (ret < 0) {
        std::cerr << "avcodec_open2 failed : " << av_get_err(ret) << "\n";
        return -1;
    }

    AVPacket pkt{};
    AVFrame frame{};

    for (;;) {
        ret = av_read_frame(format_ctx,&pkt);
        if (ret < 0) {
            std::cerr << "av_read_frame failed : " << av_get_err(ret) << "\n";
            break;
        }
        decode(*codec_ctx,pkt,frame,out_file);
    }

    decode(*codec_ctx,{},frame,out_file);

    std::cout << "read file finish\n";

    return 0;
}
