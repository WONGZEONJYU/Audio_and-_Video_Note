#include <iostream>
#include <fstream>

extern "C" {
#include <libavutil/log.h>
#include <libavformat/avio.h>
#include <libavformat/avformat.h>
#include <libavcodec/bsf.h>
}

using namespace std;

static char* av_get_err(const int errnum)
{
    static char err_buf[128] {};
    av_strerror(errnum, err_buf, sizeof(err_buf));
    return err_buf;
}

/*
AvCodecContext->extradata[]中为nalu长度
*   codec_extradata:
*   1, 64, 0, 1f, ff, e1, [0, 18], 67, 64, 0, 1f, ac, c8, 60, 78, 1b, 7e,
*   78, 40, 0, 0, fa, 40, 0, 3a, 98, 3, c6, c, 66, 80,
*   1, [0, 5],68, e9, 78, bc, b0, 0,
*/

//ffmpeg -i 2018.mp4 -codec copy -bsf:h264_mp4toannexb -f h264 tmp.h264
//ffmpeg 从mp4上提取H264的nalu h

int main(int argc, char **argv)
{
    if(argc < 3){
        cerr << "usage inputfile outfile\n";
        return -1;
    }

    //FILE *outfp=fopen(argv[2],"wb");
    ofstream outfp(argv[2],ios::binary | ios::trunc);
    cout << "in: " << argv[1] << " , out: " << argv[2] << "\n";

    // 分配解复用器的内存，使用avformat_close_input释放
    auto ifmt_ctx {avformat_alloc_context()};
    if (!ifmt_ctx){
        outfp.close();
        cerr << "[error] Could not allocate context.\n";
        return -1;
    }

    // 根据url打开码流，并选择匹配的解复用器
    auto ret { avformat_open_input(&ifmt_ctx,argv[1], nullptr, nullptr)};
    if(ret < 0){
        outfp.close();
        cerr << "[error]avformat_open_input: " << av_get_err(ret) << "\n";
        return -1;
    }

    // 读取媒体文件的部分数据包以获取码流信息
    ret = avformat_find_stream_info(ifmt_ctx, nullptr);
    if(ret < 0){
        outfp.close();
        avformat_close_input(&ifmt_ctx);
        cerr << "[error]avformat_find_stream_info: " << av_get_err(ret) << "\n";
        return -1;
    }

    // 查找出哪个码流是video/audio/subtitles

    const auto videoindex {av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0)};
    if(videoindex < 0){
        cerr << "Didn't find a video stream.\n";
        outfp.close();
        avformat_close_input(&ifmt_ctx);
        return -1;
    }

    // 1 获取相应的比特流过滤器
    //FLV/MP4/MKV等结构中，h264需要h264_mp4toannexb处理。添加SPS/PPS等信息。
    // FLV封装时，可以把多个NALU放在一个VIDEO TAG中,结构为4B NALU长度+NALU1+4B NALU长度+NALU2+...,
    // 需要做的处理把4B长度换成00000001或者000001
    const auto bsfilter {av_bsf_get_by_name("h264_mp4toannexb")};
    AVBSFContext *bsf_ctx {};
    // 2 初始化过滤器上下文
    av_bsf_alloc(bsfilter, &bsf_ctx); //AVBSFContext;
    // 3 添加解码器属性
    avcodec_parameters_copy(bsf_ctx->par_in, ifmt_ctx->streams[videoindex]->codecpar);

    av_bsf_init(bsf_ctx);

    auto pkt {av_packet_alloc()};   // 分配数据包

    bool file_end {}; // 文件是否读取结束

    while (!file_end){

        if((ret = av_read_frame(ifmt_ctx, pkt)) < 0){
            // 没有更多包可读
            file_end = true;
            cout << "read file end: ret: " << ret << "\n";
        }

        if((!ret) && (videoindex == pkt->stream_index)){
#if 0
            const auto input_size {pkt->size};

            if (av_bsf_send_packet(bsf_ctx, pkt)) {    // bitstreamfilter内部去维护内存空间
                av_packet_unref(pkt);   // 你不用了就把资源释放掉
                continue;       // 继续送
            }

            av_packet_unref(pkt);   // 释放资源

            int out_pkt_count {};/*用于统计一个packet是否含有多个NALU,有些视频把SEI SPS PPS I帧放在同一个packet*/

            while(!av_bsf_receive_packet(bsf_ctx, pkt)){/*自动加入start code*/

                ++out_pkt_count;

                cout << "write pkt size: " << pkt->size << "\n" << flush;

                const auto begin_size{outfp.tellp()};

                outfp.write(reinterpret_cast<char*>(pkt->data),pkt->size);

                const auto size {outfp.tellp() - begin_size};

                if(size != pkt->size){
                    cerr << "write failed-> write: " << size << ", pkt_size : " << pkt->size << "\n";
                }

                av_packet_unref(pkt);
            }

            if(out_pkt_count >= 2){
                cout << "cur pkt(size: " << input_size << ") only get 1 out pkt, it get " << out_pkt_count << " pkts\n";
            }
#else       // TS流可以直接写入
            const auto begin_size{outfp.tellp()};

            outfp.write(reinterpret_cast<char*>(pkt->data),pkt->size);

            const auto size {outfp.tellp()-begin_size};

            if(size != pkt->size){
                cerr << "write failed-> write: " << size << ", pkt_size : " << pkt->size << "\n";
            }

            av_packet_unref(pkt);
#endif
        }else{
            if(!ret){
                av_packet_unref(pkt);        // 释放内存
            }
        }
    }

    outfp.close();

    if(bsf_ctx){
        av_bsf_free(&bsf_ctx);
    }

    if(pkt){
        av_packet_free(&pkt);
    }

    if(ifmt_ctx){
        avformat_close_input(&ifmt_ctx);
    }

    cout << "finish\n";

    return 0;
}
