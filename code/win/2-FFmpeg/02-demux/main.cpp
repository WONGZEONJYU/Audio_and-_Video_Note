#include <iostream>
#include <string>

extern "C"{
#include <libavformat/avformat.h>
}

using namespace std;

int main(int argc,const char* argv[])
{
    cout << "begin\n";

    if(argc < 2){
        cerr << "parm less\n";
        return -1;
    }

    const string in_filename(argv[1]);
    cout << "in_filename = " << in_filename << "\n";

    //AVFormatContext是描述一个媒体文件或媒体流的构成和基本信息的结构体
    AVFormatContext *ifmt_ctx{};           // 输入文件的demux

    int videoindex{-1},        // 视频索引
        audioindex{-1};        // 音频索引

    // 打开文件,主要是探测协议类型,如果是网络文件则创建网络链接
    auto ret {avformat_open_input(&ifmt_ctx, in_filename.c_str(), nullptr, nullptr)};

    if(ret < 0){
        char errbuf[1024]{};
        av_strerror(ret, errbuf, sizeof(errbuf) - 1);
        cout << "open " << in_filename << "failed : " << errbuf << "\n";
        return -1;
    }

    ret = avformat_find_stream_info(ifmt_ctx,nullptr);

    if(ret < 0){
        char errbuf[1024]{};
        av_strerror(ret, errbuf, sizeof(errbuf) - 1);
        cout << "avformat_find_stream_info " << in_filename << "failed : " << errbuf << "\n";
    }

    cout << "\n==== av_dump_format in_filename: " << in_filename << "===\n";

    av_dump_format(ifmt_ctx, 0, in_filename.c_str(), 0);



    if(ifmt_ctx){
        avformat_close_input(&ifmt_ctx);
    }

    cout << "end\n";

    return 0;
}
