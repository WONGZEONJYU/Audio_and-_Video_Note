#include <iostream>
#include <fstream>
#include <string>
#include <memory_resource>

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/pixfmt.h>
#include <libavutil/pixdesc.h>
}

using namespace std;

static std::string av_get_err(const int& errnum)
{
    char err_buf[1024]{};
    av_strerror(errnum, err_buf, sizeof(err_buf));
    return {err_buf};
}

static void print_video_format(const AVFrame &frame)
{
    std::cout << "width: " << frame.width << "\n";
    std::cout << "height: "<< frame.height << "\n";
    std::cout << "format: " << frame.format << " " << av_get_pix_fmt_name(static_cast<AVPixelFormat>(frame.format)) << "\n";

    // 格式需要注意,实际存储到本地文件时已经改成交错模式
}

static void decode(AVCodecContext &dec_ctx,const AVPacket &pkt, AVFrame &frame,
                   ofstream &outfile)
{
    /* send the packet with the compressed data to the decoder */
    auto ret { avcodec_send_packet(&dec_ctx, &pkt)};
    if(AVERROR(EAGAIN) == ret){
        std::cerr << "Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n";
    }else if (ret < 0){
        std::cerr << "Error submitting the packet to the decoder, err: " << av_get_err(ret) <<
            " , pkt_size : " << pkt.size << "\n";
        return;
    }

    /* read all the output frames (infile general there may be any number of them */
    while (ret >= 0){
        // 对于frame, avcodec_receive_frame内部每次都先调用
        ret = avcodec_receive_frame(&dec_ctx, &frame);
        if (AVERROR(EAGAIN) == ret   || AVERROR_EOF == ret ) {
            return;
        }else if (ret < 0){
            std::cerr << "Error during decoding\n";
            exit(-1);
        }

        static bool s_print_format {};
        s_print_format = s_print_format ? true : (print_video_format(frame),true);

        // 一般H264默认为 AV_PIX_FMT_YUV420P,具体怎么强制转为 AV_PIX_FMT_YUV420P 在音视频合成输出的时候讲解
        // frame->linesize[1]  对齐的问题
        // 正确写法  linesize[]代表每行的字节数量，所以每行的偏移是linesize[]
#if 1
        for(int j{}; j<frame.height; j++) {
            const auto p{frame.data[0] + j * frame.linesize[0]};
            outfile.write(reinterpret_cast<const char *>(p),frame.width);
        }

        for(int j{}; j<frame.height/2; j++) {
            const auto p{frame.data[1] + j * frame.linesize[1]};
            outfile.write(reinterpret_cast<const char *>(p),frame.width / 2);
        }

        for(int j{}; j<frame.height/2; j++) {
            const auto p{frame.data[2] + j * frame.linesize[2]};
            outfile.write(reinterpret_cast<const char *>(p),frame.width / 2);
        }
#else
        // 错误写法 用source.200kbps.766x322_10s.h264测试时可以看出该种方法是错误的
        //  写入y分量
        outfile.write(reinterpret_cast<const char*>(frame.data[0]),frame.width * frame.height);
        // 写入u分量
        outfile.write(reinterpret_cast<const char*>(frame.data[1]),(frame.width) *(frame.height)/4);//U:宽高均是Y的一半
        //  写入v分量
        outfile.write(reinterpret_cast<const char*>(frame.data[2]),(frame.width) *(frame.height)/4);//V:宽高均是Y的一半
#endif
    }
}

/*
注册测试的时候不同分辨率的问题
提取H264: ffmpeg -i source.200kbps.768x320_10s.flv -vcodec libx264 -an -f h264 source.200kbps.768x320_10s.h264
提取MPEG2: ffmpeg -i source.200kbps.768x320_10s.flv -vcodec mpeg2video -an -f mpeg2video source.200kbps.768x320_10s.mpeg2
播放 : ffplay -pixel_format yuv420p -video_size 768x320 -framerate 25 source.200kbps.768x320_10s.yuv
*/

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

int main(const int argc,const char* argv[])
{
    constexpr auto VIDEO_INBUF_SIZE {20480};
    constexpr auto VIDEO_REFILL_THRESH {4096};

    if (argc <= 2){
        std::cerr << "Usage: " << argv[0] << " <input file> <output file>\n";
        return -1;
    }

    const auto filename {argv[1]};
    const auto outfilename {argv[2]};

    ifstream in_file(filename,ios::binary);
    ofstream out_file(outfilename,ios::binary);

    pmr::unsynchronized_pool_resource mpool;

    AVCodecParserContext *parser{};
    AVCodecContext *codec_ctx{};
    void *inbuf{};

    Destroyer d(std::move([&]() {
        in_file.close();
        out_file.close();
        avcodec_free_context(&codec_ctx);
        av_parser_close(parser);
        mpool.deallocate(inbuf,VIDEO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE);
        mpool.release();
    }));

    auto video_codec_id {AV_CODEC_ID_H264};
    if(std::string(filename).find("264") != std::string::npos){
        video_codec_id = AV_CODEC_ID_H264;
    }else if(std::string(filename).find("mpeg2") != std::string::npos){
        video_codec_id = AV_CODEC_ID_MPEG2VIDEO;
    }else{
        std::cout << "default codec id: " << video_codec_id << "\n";
    }

    /*查找解码器*/
    const auto codec{avcodec_find_decoder(video_codec_id)}; /*AV_CODEC_ID_H264*/
    if (!codec) {
        std::cerr << "Codec not found\n";
        return -1;
    }

   //获取裸流的解析器 AVCodecParserContext(数据)  +  AVCodecParser(方法);
    parser = av_parser_init(codec->id);
    if (!parser) {
        std::cerr << "Parser not found\n";
        return -1;
    }

    // 分配codec上下文
    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        std::cerr << "Could not allocate audio codec context\n";
        return -1;
    }

    if (avcodec_open2(codec_ctx,codec,nullptr) < 0) {
        std::cerr << "Could not open codec\n";
        return -1;
    }

    if (!in_file) {
        std::cerr << "in file open faild\n";
        return -1;
    }

    if (!out_file) {
        std::cerr << "out file open faild\n";
        return -1;
    }

    try {
        inbuf = mpool.allocate(VIDEO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE);
    } catch (const std::exception &e) {
        std::cerr << "mpool.allocate failed!" << e.what() << "\n";
        return -1;
    }

    auto data{static_cast<uint8_t*>(inbuf)};
    in_file.read(static_cast<char*>(inbuf),VIDEO_INBUF_SIZE);
    auto read_data_size{in_file.gcount()};

    AVPacket pkt{};
    AVFrame decoded_frame{};

    while (read_data_size > 0) {

        const auto parser_len{av_parser_parse2(parser,codec_ctx,&pkt.data,&pkt.size,
            data,read_data_size,AV_NOPTS_VALUE,AV_NOPTS_VALUE,0)};
        if (parser_len < 0) {
            std::cerr << "Error while parsing\n";
            return -1;
        }

        data += parser_len;
        read_data_size -= parser_len;

        if (pkt.size) {
            decode(*codec_ctx,pkt,decoded_frame,out_file);
        }

        if (read_data_size < VIDEO_REFILL_THRESH) {
            std::move(data,data + read_data_size,static_cast<uint8_t*>(inbuf));
            data = static_cast<uint8_t *>(inbuf);
            in_file.read(reinterpret_cast<char *>(data + read_data_size) , VIDEO_INBUF_SIZE - read_data_size);
            const auto len{in_file.gcount()};
            if (len > 0) {
                read_data_size += len;
            }
        }
    }

    pkt.data = nullptr;
    pkt.size = 0;
    decode(*codec_ctx,pkt,decoded_frame,out_file);

    std::cout << "main finish, please enter Enter and exit\n";
    return 0;
}
