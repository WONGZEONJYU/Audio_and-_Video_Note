#include <iostream>
#include <fstream>
#include <string>
#include <memory>

extern "C"{
#include <libavcodec/avcodec.h>
}

using namespace std;

static std::string av_get_err(const int errnum)
{
    char err_buf[1024]{};
    av_strerror(errnum, err_buf, sizeof(err_buf));
    return {err_buf};
}

static void print_sample_format(const AVFrame &frame)
{
    std::cout << "width: " << frame.width << "\n";
    std::cout << "height: "<< frame.height << "\n";
    std::cout << "format: " << frame.format << " " << av_get_sample_fmt_name(static_cast<AVSampleFormat>(frame.format)) << "\n";
    // 格式需要注意,实际存储到本地文件时已经改成交错模式
}

static void decode(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *frame,
                   FILE *outfile)
{
    int ret;
    /* send the packet with the compressed data to the decoder */
    ret = avcodec_send_packet(dec_ctx, pkt);
    if(ret == AVERROR(EAGAIN))
    {
        fprintf(stderr, "Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n");
    }
    else if (ret < 0)
    {
        fprintf(stderr, "Error submitting the packet to the decoder, err:%s, pkt_size:%d\n",
                av_get_err(ret), pkt->size);
        return;
    }

    /* read all the output frames (infile general there may be any number of them */
    while (ret >= 0)
    {
        // 对于frame, avcodec_receive_frame内部每次都先调用
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0)
        {
            fprintf(stderr, "Error during decoding\n");
            exit(1);
        }
        static int s_print_format = 0;
        if(s_print_format == 0)
        {
            s_print_format = 1;
            print_video_format(frame);
        }

        // 一般H264默认为 AV_PIX_FMT_YUV420P, 具体怎么强制转为 AV_PIX_FMT_YUV420P 在音视频合成输出的时候讲解
        // frame->linesize[1]  对齐的问题
        // 正确写法  linesize[]代表每行的字节数量，所以每行的偏移是linesize[]
        for(int j=0; j<frame->height; j++)
            fwrite(frame->data[0] + j * frame->linesize[0], 1, frame->width, outfile);
        for(int j=0; j<frame->height/2; j++)
            fwrite(frame->data[1] + j * frame->linesize[1], 1, frame->width/2, outfile);
        for(int j=0; j<frame->height/2; j++)
            fwrite(frame->data[2] + j * frame->linesize[2], 1, frame->width/2, outfile);

        // 错误写法 用source.200kbps.766x322_10s.h264测试时可以看出该种方法是错误的
        //  写入y分量
//        fwrite(frame->data[0], 1, frame->width * frame->height,  outfile);//Y
//        // 写入u分量
//        fwrite(frame->data[1], 1, (frame->width) *(frame->height)/4,outfile);//U:宽高均是Y的一半
//        //  写入v分量
//        fwrite(frame->data[2], 1, (frame->width) *(frame->height)/4,outfile);//V：宽高均是Y的一半
    }
}


int main(const int argc,const char* argv[])
{


    return 0;
}
