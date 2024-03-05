#include <iostream>
#include <string>
#include <fstream>
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
    std::cout << "f-format: " << frame->format << "\n";
    // 格式需要注意,实际存储到本地文件时已经改成交错模式
}

static void decode(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *frame,
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

// 播放范例: ffplay -ar 48000 -ac 2 -f f32le believe.pcm
int main(const int argc,const char* argv[])
{
    if (argc <= 2){
        std::cerr << "Usage: " << argv[0] << "<input file> <output file>\n";
        return -1;
    }

    const auto filename {argv[1]};
    const auto outfilename {argv[2]};

    auto pkt {av_packet_alloc()};
    auto audio_codec_id {AV_CODEC_ID_AAC};

    // if (string(filename).find("aac") != std::string::npos){
    //     audio_codec_id = AV_CODEC_ID_AAC;
    // }else if (string(filename).find("mp3") != std::string::npos){
    //
    // }

    av_packet_free(&pkt);

    return 0;
}
