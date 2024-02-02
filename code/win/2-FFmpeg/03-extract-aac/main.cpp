#include <iostream>
#include <fstream>

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

using namespace std;

static inline constexpr int sampling_frequencies[] {
    96000,  // 0x0
    88200,  // 0x1
    64000,  // 0x2
    48000,  // 0x3
    44100,  // 0x4
    32000,  // 0x5
    24000,  // 0x6
    22050,  // 0x7
    16000,  // 0x8
    12000,  // 0x9
    11025,  // 0xa
    8000   // 0xb
    // 0xc d e f是保留的
};

static inline constexpr int ADTS_HEADER_LEN {7};

int adts_header(char * const p_adts_header, const int data_length,
                const int profile, const int samplerate,const int channels)
{
    int sampling_frequency_index {3}; // 默认使用48000hz
    int adtsLen {data_length + ADTS_HEADER_LEN};

    constexpr int frequencies_size {sizeof(sampling_frequencies) / sizeof(sampling_frequencies[0])};

    int i{};

    for(; i < frequencies_size; i++){
        if(sampling_frequencies[i] == samplerate) {
            sampling_frequency_index = i;
            break;
        }
    }

    if(i >= frequencies_size) {
        cout << "unsupport samplerate: " << samplerate << "\n";
        return -1;
    }

    p_adts_header[0] = 0xff;         //syncword:0xfff                          高8bits
    p_adts_header[1] = 0xf0;         //syncword:0xfff                          低4bits
    p_adts_header[1] |= (0 << 3);    //MPEG Version:0 for MPEG-4,1 for MPEG-2  1bit
    p_adts_header[1] |= (0 << 1);    //Layer:0                                 2bits
    p_adts_header[1] |= 1;           //protection absent:1                     1bit

    p_adts_header[2] = (profile) << 6;            //profile:profile               2bits
    p_adts_header[2] |= (sampling_frequency_index & 0x0f) << 2; //sampling frequency index:sampling_frequency_index  4bits
    p_adts_header[2] |= (0 << 1);             //private bit:0                   1bit
    p_adts_header[2] |= (channels & 0x04)>>2; //channel configuration:channels  高1bit

    p_adts_header[3] = (channels & 0x03)<<6; //channel configuration:channels 低2bits
    p_adts_header[3] |= (0 << 5);               //original：0                1bit
    p_adts_header[3] |= (0 << 4);               //home：0                    1bit
    p_adts_header[3] |= (0 << 3);               //copyright id bit：0        1bit
    p_adts_header[3] |= (0 << 2);               //copyright id start：0      1bit
    p_adts_header[3] |= ((adtsLen & 0x1800) >> 11);           //frame length：value   高2bits

    p_adts_header[4] = static_cast<uint8_t>((adtsLen & 0x7f8) >> 3);     //frame length:value    中间8bits
    p_adts_header[5] = static_cast<uint8_t>((adtsLen & 0x7) << 5);       //frame length:value    低3bits
    p_adts_header[5] |= 0x1f;                                 //buffer fullness:0x7ff 高5bits
    p_adts_header[6] = 0xfc;      //‭11111100‬       //buffer fullness:0x7ff 低6bits
    // number_of_raw_data_blocks_in_frame：
    //    表示ADTS帧中有number_of_raw_data_blocks_in_frame + 1个AAC原始帧。

    return 0;
}

int main(int argc, const char *argv[])
{
    // 设置打印级别
    //av_log_set_level(AV_LOG_DEBUG);

    if(argc < 3){
        cerr << "the count of parameters should be more than three!\n";
        return -1;
    }

    const auto in_filename{argv[1]},aac_filename{argv[2]};

    if(!in_filename || !aac_filename){
        cerr << "src or dts file is null, plz check them!\n";
        return -1;
    }

    ofstream acc_out_filename(aac_filename,ios::binary | ios::trunc);

    if(!acc_out_filename){
        cerr << "Could not open destination file " << aac_filename << "\n";
        return -1;
    }

    AVFormatContext *ifmt_ctx {};

    int ret{-1};
    // 打开输入文件
    if((ret = avformat_open_input(&ifmt_ctx, in_filename, nullptr, nullptr)) < 0) {
        //cerr << "Could not open source file: " << in_filename << "\n";
        char errors[1024]{};
        av_strerror(ret, errors, sizeof(errors));
        av_log(NULL, AV_LOG_DEBUG, "Could not open source file: %s, %d(%s)\n",
               in_filename,
               ret,
               errors);
        return -1;
    }

    // 获取解码器信息

    if((ret = avformat_find_stream_info(ifmt_ctx,nullptr)) < 0) {
        //cerr << "failed to find stream information: <<" << in_filename << "\n";
        char errors[1024]{};
        av_strerror(ret, errors, sizeof(errors));
        av_log(NULL, AV_LOG_DEBUG, "failed to find stream information: %s, %d(%s)\n",
               in_filename,
               ret,
               errors);
        return -1;
    }

    // dump媒体信息
    av_dump_format(ifmt_ctx, 0, in_filename, 0);

    // 初始化packet
    //av_init_packet(&pkt);
    AVPacket pkt{};

    //查找audio对应的steam index
    const auto audio_index {av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0)};
    if(audio_index < 0){

//        cerr << "Could not find "<< av_get_media_type_string(AVMEDIA_TYPE_AUDIO) <<
//                " stream in input file " << in_filename << "\n";

        av_log(nullptr, AV_LOG_DEBUG, "Could not find %s stream in input file %s\n",
               av_get_media_type_string(AVMEDIA_TYPE_AUDIO),
               in_filename);

        return AVERROR(EINVAL);
    }

    // 打印AAC级别
    const auto profile{ifmt_ctx->streams[audio_index]->codecpar->profile};
    cout << "\n\naudio profile: " << profile << ", FF_PROFILE_AAC_LOW : " << FF_PROFILE_AAC_LOW << "\n\n";

    if(ifmt_ctx->streams[audio_index]->codecpar->codec_id != AV_CODEC_ID_AAC) {

        const auto codec_id {ifmt_ctx->streams[audio_index]->codecpar->codec_id};
        cout << "the media file no contain AAC stream, it's codec_id is " << codec_id << "\n";
        if(ifmt_ctx) {
            avformat_close_input(&ifmt_ctx);
        }

        acc_out_filename.close();
    }

#if 0
    constexpr int64_t start_time_us{191000000},end_time_us{441000000};

    const auto start_time {av_rescale_q(start_time_us, AV_TIME_BASE_Q, ifmt_ctx->streams[audio_index]->time_base)},
                end_time {av_rescale_q(end_time_us, AV_TIME_BASE_Q, ifmt_ctx->streams[audio_index]->time_base)};

    cout << "start_time = " << start_time << "\n";
    cout << "end_time = " << end_time << "\n";

    const auto r {avformat_seek_file(ifmt_ctx,audio_index,0,start_time,end_time,0)};

    if(r < 0){
        cerr << "avformat_seek_file faild\n";
        if(ifmt_ctx) {
            avformat_close_input(&ifmt_ctx);
        }

        acc_out_filename.close();
        return -1;
    }

#endif
    //读取媒体文件,并把aac数据帧写入到本地文件
    while(av_read_frame(ifmt_ctx, &pkt) >=0 ) {
        if(audio_index == pkt.stream_index) {
            char adts_header_buf[7] {};
            adts_header(adts_header_buf, pkt.size,
                        ifmt_ctx->streams[audio_index]->codecpar->profile,
                        ifmt_ctx->streams[audio_index]->codecpar->sample_rate,
                        ifmt_ctx->streams[audio_index]->codecpar->ch_layout.nb_channels);

            // 写adts header ,ts流不适用,ts流分离出来的packet带了adts header
            acc_out_filename.write(adts_header_buf,sizeof(adts_header_buf));

            const auto start_pos {acc_out_filename.tellp() };
            acc_out_filename.write(reinterpret_cast<const char*>(pkt.data),pkt.size);
            const auto len {acc_out_filename.tellp() - start_pos};

            if(len != pkt.size) {
                cout << "warning, length of writed data isn't equal pkt.size(" << len << ", "<< pkt.size << ")\n";
            }
        }

        av_packet_unref(&pkt);
    }

    // 关闭输入文件
    if(ifmt_ctx) {
        avformat_close_input(&ifmt_ctx);
    }

    acc_out_filename.close();

    return 0;
}
