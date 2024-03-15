#include <iostream>
#include <string>
#include <fstream>

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/bsf.h>
#include <libavutil/log.h>
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

static std::string av_get_err(const int& errnum)
{
    constexpr auto ERROR_STRING_SIZE {1024};
    char err_buf[ERROR_STRING_SIZE]{};
    av_strerror(errnum, err_buf, sizeof(err_buf));
    return {err_buf};
}

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

static inline constexpr auto ADTS_HEADER_LEN {7};

static int adts_header(uint8_t * const p_adts_header, const int &data_length,
                const int &profile, const int &samplerate,const int &channels)
{

#if 0
    constexpr auto frequencies_size {std::size(sampling_frequencies)};
    int i{},sampling_frequency_index {3};

    for(; i < frequencies_size; i++){
        if(sampling_frequencies[i] == samplerate) {
            sampling_frequency_index = i;
            break;
        }
    }

    if(i >= frequencies_size) {
        std::cout << "unsupport samplerate: " << samplerate << "\n";
        return -1;
    }
#else
    auto sampling_frequency_index{-1};
    for(const auto& v : sampling_frequencies){
        if (samplerate == v){
            sampling_frequency_index = static_cast<int>(&v - sampling_frequencies);
        }
    }
    if(sampling_frequency_index < 0){
        std::cout << "unsupport samplerate: " << samplerate << "\n";
        return -1;
    }
#endif

    const auto adtsLen {data_length + ADTS_HEADER_LEN};

    p_adts_header[0] = 0xff;         //syncword:0xfff                          高8bits
    p_adts_header[1] = 0xf0;         //syncword:0xfff                          低4bits
    p_adts_header[1] |= 0 << 3;    //MPEG Version:0 for MPEG-4,1 for MPEG-2  1bit
    p_adts_header[1] |= 0 << 1;    //Layer:0                                 2bits
    p_adts_header[1] |= 1;           //protection absent:1                     1bit

    p_adts_header[2] = profile << 6;            //profile:profile               2bits
    p_adts_header[2] |= (sampling_frequency_index & 0x0f) << 2; //sampling frequency index:sampling_frequency_index  4bits
    p_adts_header[2] |= 0 << 1;             //private bit:0                   1bit
    p_adts_header[2] |= (channels & 0x04)>>2; //channel configuration:channels  高1bit

    p_adts_header[3] = (channels & 0x03)<<6; //channel configuration:channels 低2bits
    p_adts_header[3] |= 0 << 5;               //original：0                1bit
    p_adts_header[3] |= 0 << 4;               //home：0                    1bit
    p_adts_header[3] |= 0 << 3;               //copyright id bit：0        1bit
    p_adts_header[3] |= 0 << 2;               //copyright id start：0      1bit
    p_adts_header[3] |= (adtsLen & 0x1800) >> 11;           //frame length：value   高2bits

    p_adts_header[4] = static_cast<uint8_t>((adtsLen & 0x7f8) >> 3);     //frame length:value    中间8bits
    p_adts_header[5] = static_cast<uint8_t>((adtsLen & 0x7) << 5);       //frame length:value    低3bits
    p_adts_header[5] |= 0x1f;                                 //buffer fullness:0x7ff 高5bits
    p_adts_header[6] = 0xfc;      //‭11111100‬       //buffer fullness:0x7ff 低6bits
    // number_of_raw_data_blocks_in_frame：
    //    表示ADTS帧中有number_of_raw_data_blocks_in_frame + 1个AAC原始帧。

    return 0;
}

static int video_handler_helper(AVBSFContext &bsf_ctx,AVPacket &pkt,std::ofstream &out_vedio_file)
{
    auto ret{av_bsf_send_packet(&bsf_ctx,&pkt)};

    if (ret < 0){
        std::cerr << "av_bsf_send_packet failed: " << av_get_err(ret) << "\n";
        av_packet_unref(&pkt);
        return ret;
    }

    for (;;){
        if((ret = av_bsf_receive_packet(&bsf_ctx, &pkt)) < 0){
            std::cerr << "av_bsf_receive_packet : " << av_get_err(ret) << "\n";
            break;
        }
        const auto prv_len{out_vedio_file.tellp()};
        out_vedio_file.write(reinterpret_cast<const char*>(pkt.data),pkt.size);
        const auto size{out_vedio_file.tellp() - prv_len};

        if(size != pkt.size){
            av_log(nullptr, AV_LOG_DEBUG, "h264 warning, length of writed data isn't equal pkt->size(%d, %d)\n",
                   static_cast<int>(size),
                   pkt.size);
        }
        av_packet_unref(&pkt);
    }

    return {};
}

static int audio_handler_helper(const AVFormatContext &ifmt_ctx,AVPacket &pkt,std::ofstream &out_audio_file)
{
    const auto profile{ifmt_ctx.streams[pkt.stream_index]->codecpar->profile}
                ,sample_rate{ifmt_ctx.streams[pkt.stream_index]->codecpar->sample_rate}
                ,channels{ifmt_ctx.streams[pkt.stream_index]->codecpar->ch_layout.nb_channels};

    uint8_t adts_header_buf[ADTS_HEADER_LEN]{};
    adts_header(adts_header_buf,pkt.size,profile,sample_rate,channels);

    // 写adts header,ts流不适用,ts流分离出来的packet带了adts header
    out_audio_file.write(reinterpret_cast<const char*>(adts_header_buf),std::size(adts_header_buf));

    const auto prv_len{out_audio_file.tellp()};
    out_audio_file.write(reinterpret_cast<const char*>(pkt.data),pkt.size);
    const auto size{out_audio_file.tellp() - prv_len};

    if (size != pkt.size){
        av_log(nullptr, AV_LOG_DEBUG, "aac warning, length of writed data isn't equal pkt->size(%d, %d)\n",
       static_cast<int>(size),
       pkt.size);
    }

    av_packet_unref(&pkt);
    return {};
}

int main(const int argc,const char* argv[]) {
    // 判断参数
    if(argc < 4) {
        std::cerr << "usage app input.mp4  out.h264 out.aac\n";
        return -1;
    }

    const auto in_file{argv[1]};
    std::ofstream out_h264_file(argv[2],std::ios::binary),
                out_aac_file(argv[3],std::ios::binary);

    AVFormatContext *ifmt_ctx {};
    AVBSFContext *bsf_ctx{};
    AVPacket pkt{};
    av_packet_unref(&pkt);/*此处用于初始化AVPacket av_init_packet(...)函数官方已经弃用*/

    auto rres{[&]() {
        out_h264_file.close();
        out_aac_file.close();
        av_packet_unref(&pkt);
        avformat_close_input(&ifmt_ctx);
        av_bsf_free(&bsf_ctx);
    }};

    Destroyer d(std::move(rres));

    if (!out_h264_file) {
        std::cerr << "open out_h264_file failed\n";
        return -1;
    }

    if (!out_aac_file) {
        std::cerr << "open out_aac_file\n";
        return -1;
    }

    ifmt_ctx = avformat_alloc_context();
    if (!ifmt_ctx) {
        std::cerr << "avformat_alloc_context failed\n";
        return -1;
    }

    auto ret{avformat_open_input(&ifmt_ctx,in_file,nullptr,nullptr)};

    if (ret < 0) {
        std::cerr << "avformat_open_input failed: " << av_get_err(ret) << "\n";
        return -1;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx,nullptr)) < 0) {
        std::cerr << "avformat_find_stream_info faild : " << av_get_err(ret) << "\n";
        return -1;
    }

    const auto video_index{av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0)};
    if(video_index < 0) {
        std::cerr << "av_find_best_stream video_index failed\n";
        return -1;
    }

    const auto audio_index {av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0)};
    if(audio_index < 0) {
        std::cerr << "av_find_best_stream audio_index failed\n";
        return -1;
    }

    const auto bsfilter{av_bsf_get_by_name("h264_mp4toannexb")};
    if (!bsfilter){
        std::cerr << "av_bsf_get_by_name h264_mp4toannexb failed\n";
        return -1;
    }

    if ((ret = av_bsf_alloc(bsfilter,&bsf_ctx)) < 0){
        std::cerr << "av_bsf_alloc filad : " << av_get_err(ret) << "\n";
        return -1;
    }

    ret = avcodec_parameters_copy(bsf_ctx->par_in, ifmt_ctx->streams[video_index]->codecpar);
    if (ret < 0){
        std::cerr << "avcodec_parameters_copy failed: " << av_get_err(ret) << "\n";
        return -1;
    }

    if ((ret = av_bsf_init(bsf_ctx)) < 0){
        std::cerr << "av_bsf_init failed: " << av_get_err(ret) << "\n";
        return -1;
    }

    for(;;){

        if ((ret = av_read_frame(ifmt_ctx,&pkt)) < 0) {  // 不会去释放pkt的buf，如果我们外部不去释放，就会出现内存泄露
            std::cerr << "av_read_frame failed: " << av_get_err(ret) << "\n";
            break;
        }
        // av_read_frame 成功读取到packet,则外部需要进行buf释放
        if (video_index == pkt.stream_index){
            video_handler_helper(*bsf_ctx,pkt,out_h264_file);
        }else if (audio_index == pkt.stream_index){
            audio_handler_helper(*ifmt_ctx,pkt,out_aac_file);
        }else{
            av_packet_unref(&pkt);
        }
    }

    std::cout << "finish demux\n" << std::flush;

    return 0;
}
