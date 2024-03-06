#include <iostream>
#include <string>
#include <fstream>
#include <bits/ranges_algobase.h>

extern "C" {
#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>
#include <libavutil/error.h>
}

using namespace std;

static inline auto AUDIO_INBUF_SIZE {40280};
static inline auto AUDIO_REFILL_THRESH {4096};

static std::string av_get_err(const int errnum)
{
    char err_buf[1024]{};
    av_strerror(errnum, err_buf, sizeof(err_buf));
    return {err_buf};
}

static void print_sample_format(const AVFrame *const frame)
{
    std::cout << "ar-samplerate: " << frame->sample_rate << "Hz\n";
    std::cout << "ac-channel: "<< frame->ch_layout.nb_channels << "\n";
    std::cout << "f-format: " << frame->format << " " << av_get_sample_fmt_name(static_cast<AVSampleFormat>(frame->format)) << "\n";
    // 格式需要注意,实际存储到本地文件时已经改成交错模式
}

static void decode(AVCodecContext *dec_ctx,const AVPacket *pkt, AVFrame *frame,
                   ofstream& outfile)
{
    /* send the packet with the compressed data to the decoder */
    auto ret = avcodec_send_packet(dec_ctx, pkt);
    if(AVERROR(EAGAIN) == ret){
        std::cerr << "Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n";
    }else if (ret < 0){
        std::cout << "Error submitting the packet to the decoder, err: " << av_get_err(ret) << " , pkt_size: " << pkt->size << "\n";
        return;
    }else{

    }

    /* read all the output frames (infile general there may be any number of them */
    while (ret >= 0) {
        // 对于frame, avcodec_receive_frame内部每次都先调用
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (AVERROR(EAGAIN) == ret || ret == AVERROR_EOF){
            return;
        }else if (ret < 0){
            std::cerr << "Error during decoding\n";
            exit(ret);
        }
        //std::cout << av_get_sample_fmt_name(dec_ctx->sample_fmt) << "\n";
        const auto data_size {av_get_bytes_per_sample(dec_ctx->sample_fmt)};
        if (data_size < 0){
            /* This should not occur, checking just for paranoia */
            std::cout << "Failed to calculate data size\n";
            exit(-1);
        }

        static bool s_print_format {};
        s_print_format = s_print_format ? s_print_format : true,print_sample_format(frame);

        /**
            P表示Planar(平面),其数据格式排列方式为:
            LLLLLLRRRRRRLLLLLLRRRRRRLLLLLLRRRRRRL...(每个LLLLLLRRRRRR为一个音频帧)
            而不带P的数据格式(即交错排列)排列方式为:
            LRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLRL...(每个LR为一个音频样本)
            播放范例: ffplay -ar 48000 -ac 2 -f f32le believe.pcm
          **/
        for (int i {}; i < frame->nb_samples; i++){
            for (int ch {}; ch < dec_ctx->ch_layout.nb_channels; ch++){
                //交错的方式写入,大部分float的格式输出
                //fwrite(frame->data[ch] + data_size * i, 1, data_size, outfile);
                outfile.write(reinterpret_cast<const char*>(frame->data[ch] + data_size * i),data_size);
            }
        }
    }
}

template<typename T>
struct destory_ final{
    explicit destory_(T&& f):f(std::move(f)){}
    ~destory_(){f();}
private:
    T f;
};

// 播放范例: ffplay -ar 48000 -ac 2 -f f32le believe.pcm
int main(const int argc,const char* argv[])
{
    if (argc <= 2){
        std::cerr << "Usage: " << argv[0] << "<input file> <output file>\n";
        return -1;
    }

    const auto filename {argv[1]};
    const auto outfilename {argv[2]};
    AVPacket* pkt{};
    AVCodecParserContext *parser{};
    AVCodecContext *codec_ctx{};
    AVFrame *decoded_frame{};
    ifstream in_file;
    ofstream out_file;

    auto destory{[&](){
        std::cout << "destory()\n";
        in_file.close();
        out_file.close();
        av_parser_close(parser);
        avcodec_free_context(&codec_ctx);
        av_frame_free(&decoded_frame);
        av_packet_free(&pkt);
    }};

    destory_ d(std::move(destory));

    auto audio_codec_id {AV_CODEC_ID_AAC};

    if (string(filename).find("aac") != std::string::npos){
        audio_codec_id = AV_CODEC_ID_AAC;
    }else if (string(filename).find("mp3") != std::string::npos){
        audio_codec_id = AV_CODEC_ID_MP3;
    }else{
        std::cout << "default codec id:" << audio_codec_id << "\n";
    }

    // 查找解码器
    const auto codec { avcodec_find_decoder(audio_codec_id)};  // AV_CODEC_ID_AAC
    if (!codec) {
        std::cerr << "Codec not found\n";
        return -1;
    }

    // 获取裸流的解析器 AVCodecParserContext(数据)  +  AVCodecParser(方法)
    parser = av_parser_init(codec->id);
    if (!parser) {
        std::cerr << "Parser not found\n";
        return -1;
    }

    // 分配codec上下文
    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        std::cout << "Could not allocate audio codec context\n";
        return -1;
    }

    if(avcodec_open2(codec_ctx,codec,nullptr) < 0){
        std::cerr << "Could not open codec\n";
        return -1;
    }

    /*open input file*/
    in_file.open(filename,ios::binary);
    if (!in_file){
        std::cerr << "open input file faild\n";
        return -1;
    }
    /*open output file*/
    out_file.open(outfilename,ios::binary);
    if (!out_file){
        std::cerr << "open output file faild\n";
        return -1;
    }

    pkt = av_packet_alloc();
    if (!pkt){
        std::cerr << "av_packet_alloc faild\n";
        return -1;
    }
    decoded_frame = av_frame_alloc();
    if (!decoded_frame){
        std::cerr << "Could not allocate audio frame\n";
        return -1;
    }

    uint8_t inbuf[AUDIO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE]{};
    auto data{inbuf};
    in_file.read(reinterpret_cast<char*>(inbuf),AUDIO_INBUF_SIZE);
    auto read_data_size{in_file.tellg()};

    while (read_data_size > 0){

        const auto ret {av_parser_parse2(parser, codec_ctx, &pkt->data, &pkt->size,
                               data, read_data_size,AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0) };
        if (ret < 0){
            std::cerr << "Error while parsing\n";
            return -1;
        }

        data += ret;
        read_data_size -= ret;

        if (pkt->size){
            decode(codec_ctx,pkt,decoded_frame,out_file);
        }

        if (read_data_size < AUDIO_REFILL_THRESH){
            //std::move(data,data + read_data_size,inbuf);
            //memmove(inbuf,data,read_data_size);
            data = inbuf;
            in_file.read(reinterpret_cast<char*>(data + read_data_size),AUDIO_REFILL_THRESH - read_data_size);
            const auto len{in_file.tellg()};
            //read_data_size = len > 0 ? read_data_size + len : read_data_size;
            if (len > 0){
                read_data_size += len;
            }
        }
    }

    pkt->data = nullptr;
    pkt->size = 0;
    decode(codec_ctx,pkt,decoded_frame,out_file);

    std::cout << "main finish, please enter Enter and exit\n";
    return 0;
}
