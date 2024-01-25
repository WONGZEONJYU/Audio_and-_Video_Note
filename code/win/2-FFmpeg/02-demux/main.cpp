#include <iostream>
#include <string>

extern "C"{
#include <libavformat/avformat.h>
}

using namespace std;

static void show_vidio_info(const AVStream *in_stream)
{
    cout << "----- Video info:\n" <<
            "index: " << in_stream->index << "\n" <<
            // avg_frame_rate: 视频帧率,单位为fps，表示每秒出现多少帧
            "fps: " << av_q2d(in_stream->avg_frame_rate) << "fps\n";

    //视频压缩编码格式
    if (AV_CODEC_ID_MPEG4 == in_stream->codecpar->codec_id) {
        cout << "video codec: MPEG4\n";
    }else if (AV_CODEC_ID_H264 == in_stream->codecpar->codec_id){
        cout << "video codec: H264\n";
    }else{
        cout << "video codec_id: " << in_stream->codecpar->codec_id << "\n";
    }

    // 视频帧宽度和帧高度
    cout << "width: " << in_stream->codecpar->width << "\nheight: " << in_stream->codecpar->height << "\n";

    //视频总时长,单位为秒。注意如果把单位放大为毫秒或者微秒,音频总时长跟视频总时长不一定相等的
    if(in_stream->duration != AV_NOPTS_VALUE) {

        const int duration_video ((in_stream->duration) * av_q2d(in_stream->time_base));
        //将视频总时长转换为时分秒的格式打印到控制台上
        cout << "video duration: " << (duration_video / 3600) << ":" <<
                ((duration_video % 3600) / 60) << ":" <<
                ((duration_video % 60)) << "\n";

    }else{
        cout << "video duration unknown\n";
    }

    cout << "\n";
}

static auto show_vidio_info(AVFormatContext *ifmt_ctx)
{
    cout << "----- Video info:\n";

    const AVCodec *decoder{};

    const auto index{ av_find_best_stream(ifmt_ctx,AVMEDIA_TYPE_VIDEO,-1,-1,&decoder,0)};

    if(index < 0){
        cerr << "av_find_best_stream faild\n";
        return -1;
    }

    const auto in_stream{ifmt_ctx->streams[index]};

    cout << "index: " << index << "\nfps: " <<
            av_q2d(in_stream->avg_frame_rate) << "\nvideo codec: " <<

    //视频压缩编码格式
    (decoder ? decoder->name : "error") << "\n";
    // 视频帧宽度和帧高度
    cout << "width: " << in_stream->codecpar->width << "\nheight: " <<
            in_stream->codecpar->height << "\n";

    //视频总时长,单位为秒。注意如果把单位放大为毫秒或者微秒,音频总时长跟视频总时长不一定相等的
    if(in_stream->duration != AV_NOPTS_VALUE) {

        const int duration_video ((in_stream->duration) * av_q2d(in_stream->time_base));
        //将视频总时长转换为时分秒的格式打印到控制台上
        cout << "video duration: " << (duration_video / 3600) << ":" <<
                ((duration_video % 3600) / 60) << ":" <<
                ((duration_video % 60)) << "\n";

    }else{
        cout << "video duration unknown\n";
    }

    cout << "\n";
    return index;
}

static void show_audio_info(const AVStream *in_stream)
{
    cout << "----- Audio info:\n" <<
    // index:每个流成分在ffmpeg解复用分析后都有唯一的index作为标识
    "index: " << in_stream->index << "\n" <<
    // sample_rate: 音频编解码器的采样率，单位为Hz
    "samplerate: " << in_stream->codecpar->sample_rate << "Hz\n";

    // codecpar->format: 音频采样格式
    if(AV_SAMPLE_FMT_FLTP == in_stream->codecpar->format){
        cout << "sampleformat: AV_SAMPLE_FMT_FLTP\n";
    }else if(AV_SAMPLE_FMT_S16P == in_stream->codecpar->format){
        cout << "sampleformat:AV_SAMPLE_FMT_S16P\n";
    }else{

    }

    // channels: 音频信道数目
    cout << "channel number: " <<in_stream->codecpar->ch_layout.nb_channels << "\n";

    // codec_id: 音频压缩编码格式
    if (AV_CODEC_ID_AAC == in_stream->codecpar->codec_id) {
        cout << "audio codec:AAC\n";
    }else if (AV_CODEC_ID_MP3 == in_stream->codecpar->codec_id){
        cout << "audio codec:MP3\n";
    }else{
        cout << "audio codec_id: " << in_stream->codecpar->codec_id << "\n";
    }

    // 音频总时长,单位为秒.注意如果把单位放大为毫秒或者微秒,音频总时长跟视频总时长不一定相等的
    if(in_stream->duration != AV_NOPTS_VALUE){
        const int duration_audio ((in_stream->duration) * av_q2d(in_stream->time_base));
        //将音频总时长转换为时分秒的格式打印到控制台上

        cout << "audio duration: " << (duration_audio / 3600) << ":" <<
              ((duration_audio % 3600) / 60) << ":" << (duration_audio % 60) << "\n";

    }else{
        cout << "audio duration unknown\n";
    }

    cout << "\n";
}

static auto show_audio_info(AVFormatContext *ifmt_ctx)
{
    cout << "----- Audio info:\n";

    const AVCodec *decoder{};

    const auto index{ av_find_best_stream(ifmt_ctx,AVMEDIA_TYPE_AUDIO,-1,-1,&decoder,0)};

    if(index < 0){
        cerr << "av_find_best_stream faild\n";
        return -1;
    }

    const auto in_stream{ifmt_ctx->streams[index]};

    // index:每个流成分在ffmpeg解复用分析后都有唯一的index作为标识
    cout << "index: " << index << "\nsamplerate: " <<
    // sample_rate: 音频编解码器的采样率，单位为Hz
            in_stream->codecpar->sample_rate << "Hz\nsampleformat: " <<
    //音频采样格式
       (decoder ? decoder->name : "error") << "\n";

    // 音频总时长,单位为秒.注意如果把单位放大为毫秒或者微秒,音频总时长跟视频总时长不一定相等的
    if(in_stream->duration != AV_NOPTS_VALUE){
        const int duration_audio ((in_stream->duration) * av_q2d(in_stream->time_base));
        //将音频总时长转换为时分秒的格式打印到控制台上

        cout << "audio duration: " << (duration_audio / 3600) << ":" <<
              ((duration_audio % 3600) / 60) << ":" << (duration_audio % 60) << "\n";

    }else{
        cout << "audio duration unknown\n";
    }

    cout << "\n";
    return index;
}

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
        avformat_close_input(&ifmt_ctx);
        return -1;
    }

    cout << "\n==== av_dump_format in_filename: " << in_filename << "===\n";

    av_dump_format(ifmt_ctx, 0, in_filename.c_str(), 0);

    cout << "\n==== av_dump_format finish =======\n\n";

    // url:调用avformat_open_input读取到的媒体文件的路径/名字

    cout << "media name: " << ifmt_ctx->url << "\n" <<
            "stream number : " << ifmt_ctx->nb_streams << "\n" <<
            "media average ratio: " << static_cast<int64_t>(ifmt_ctx->bit_rate/1024) << " kbps\n";

    /*time*/

    const auto total_seconds {(ifmt_ctx->duration) / AV_TIME_BASE};  // 1000us = 1ms, 1000ms = 1秒
    const auto hour {total_seconds / 3600};
    const auto minute {(total_seconds % 3600) / 60};
    const auto second {total_seconds % 60};
    //通过上述运算,可以得到媒体文件的总时长
    cout << "total duration: " << hour << ":" << minute << ":" << second << "\n\n";

    /*
     * 老版本通过遍历的方式读取媒体文件视频和音频的信息
     * 新版本的FFmpeg新增加了函数av_find_best_stream,也可以取得同样的效果
     */

#if 0
    int videoindex{} ,// 视频索引
    audioindex{};

    for(uint32_t i{};i < ifmt_ctx->nb_streams;++i){

        const auto in_stream {ifmt_ctx->streams[i]};    //音频流、视频流、字幕流
        //如果是音频流,则打印音频的信息
        if (AVMEDIA_TYPE_VIDEO== in_stream->codecpar->codec_type){
            show_vidio_info(in_stream);
            videoindex = i;
        }else if(AVMEDIA_TYPE_AUDIO == in_stream->codecpar->codec_type){
            show_audio_info(in_stream);
            audioindex = i;
        }else{
            cout << "audio duration unknown\n";
        }
    }
#endif

#if 1
    const auto videoindex {show_vidio_info(ifmt_ctx)};// 视频索引
    const auto audioindex {show_audio_info(ifmt_ctx)};// 音频索引
#endif
    cout << "videoindex = " << videoindex << "\n" <<
            "audioindex = " << audioindex << "\n\n";

    if(ifmt_ctx){
        avformat_close_input(&ifmt_ctx);
    }

    cout << "end\n";

    return 0;
}
