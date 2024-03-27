
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
#include <libavcodec/codec_id.h>
#include <libavformat/avformat.h>
}

#include <iostream>
#include <fstream>
#include <memory_resource>

static std::string av_get_err(const int errnum)
{
    char err_buf[1024]{};
    av_strerror(errnum, err_buf, sizeof(err_buf));
    return {err_buf};
}

/* 检测该编码器是否支持该采样格式 */
static bool check_sample_fmt(const AVCodec *codec, const AVSampleFormat &sample_fmt)
{
    // 通过AV_SAMPLE_FMT_NONE作为结束符
    for(auto p {codec->sample_fmts};*p != AV_SAMPLE_FMT_NONE;++p){
        if (sample_fmt == *p){
            return true;
        }
    }

    return {};
}

/* 检测该编码器是否支持该采样率 */
static bool check_sample_rate(const AVCodec *codec, const int &sample_rate)
{
    // 0作为退出条件,比如libfdk-aacenc.c的aac_sample_rates
    for(auto p {codec->supported_samplerates};*p;++p){
        std::cout << codec->name << " support " << *p << "hz\n";
        if (*p == sample_rate){
            return true;
        }
    }

    return {};
}

/* 检测该编码器是否支持该通道布局, 该函数只是作参考 */
static bool check_channel_layout(const AVCodec *codec, const AVChannelLayout &channel_layout)
{
    // 不是每个codec都给出支持的channel_layout
    auto p{codec->ch_layouts};

    if(!p) {
        std::cout << "the codec " << codec->name << " no set channel_layouts\n";
        return true;
    }

    for(;p->u.mask;++p){
        std::cout << codec->name << " support channel_layout " << p->u.mask << "\n";
        if (p->u.mask == channel_layout.u.mask){
            return true;
        }
    }

    return {};
}

static void get_adts_header(const AVCodecContext *ctx, uint8_t *adts_header,const int &aac_length)
{
    int freq_idx {};    //0: 96000 Hz  3: 48000 Hz 4: 44100 Hz
    switch (ctx->sample_rate) {
        case 96000: freq_idx = 0; break;
        case 88200: freq_idx = 1; break;
        case 64000: freq_idx = 2; break;
        case 48000: freq_idx = 3; break;
        case 44100: freq_idx = 4; break;
        case 32000: freq_idx = 5; break;
        case 24000: freq_idx = 6; break;
        case 22050: freq_idx = 7; break;
        case 16000: freq_idx = 8; break;
        case 12000: freq_idx = 9; break;
        case 11025: freq_idx = 10; break;
        case 8000: freq_idx = 11; break;
        case 7350: freq_idx = 12; break;
        default: freq_idx = 4; break;
    }

    const auto chanCfg {ctx->ch_layout.nb_channels};
    const auto frame_length {aac_length + 7};
    adts_header[0] = 0xFF;
    adts_header[1] = 0xF1;
    adts_header[2] = (ctx->profile << 6) + (freq_idx << 2) + (chanCfg >> 2);
    adts_header[3] = ((chanCfg & 3) << 6) + (frame_length  >> 11);
    adts_header[4] = (frame_length & 0x7FF) >> 3;
    adts_header[5] = ((frame_length & 7) << 5) + 0x1F;
    adts_header[6] = 0xFC;
}

static int encode(AVCodecContext &ctx,const AVFrame &frame, AVPacket &pkt, std::ofstream &output)
{
    /* send the frame for encoding */
    auto ret {avcodec_send_frame(&ctx, &frame)};

    if (ret < 0) {
        std::cout << "Error sending the frame to the encoder\n";
        return -1;
    }

    /* read all the available output packets (in general there may be any number of them */
    // 编码和解码都是一样的,都是send 1次,然后receive多次, 直到AVERROR(EAGAIN)或者AVERROR_EOF
    while (ret >= 0) {
        ret = avcodec_receive_packet(&ctx, &pkt);
        if (AVERROR(EAGAIN) == ret || AVERROR_EOF == ret ) {
            return 0;
        } else if (ret < 0) {
            std::cerr << "Error encoding audio frame\n";
            return -1;
        }else{

        }

        //printf("ctx->flags:0x%x & AV_CODEC_FLAG_GLOBAL_HEADER:0x%x, name:%s\n",ctx.flags, ctx.flags & AV_CODEC_FLAG_GLOBAL_HEADER, ctx.codec->name);
        const auto has_header{ctx.flags & AV_CODEC_FLAG_GLOBAL_HEADER};
        std::cout << "ctx->flags:0x" << std::hex << ctx.flags << " & AV_CODEC_FLAG_GLOBAL_HEADER:0x" <<
            std::hex << has_header << ", name:" << ctx.codec->name << "\n";

        if(has_header) {
            // 需要额外的adts header写入
            uint8_t aac_header[7]{};
            get_adts_header(&ctx, aac_header, pkt.size);
            const auto prv_len{output.tellp()};
            //len = fwrite(aac_header, 1, 7, output);
            output.write(reinterpret_cast<const char*>(aac_header),7);
            //const auto len{output.tellp() - prv_len};
            if( 7 != output.tellp() - prv_len ) {
                std::cerr << "fwrite aac_header failed\n";
                return -1;
            }
        }

        const auto prv_len{output.tellp()};
        output.write(reinterpret_cast<const char*>(pkt.data),pkt.size);
        //const auto len{output.tellp() - prv_len};
        if(output.tellp() - prv_len != pkt.size) {
            std::cerr << "fwrite aac data failed\n";
            return -1;
        }

        /* 是否需要释放数据?avcodec_receive_packet第一个调用的就是 av_packet_unref
        * 所以我们不用手动去释放,这里有个问题,不能将pkt直接插入到队列,因为编码器会释放数据
        * 可以新分配一个pkt, 然后使用av_packet_move_ref转移pkt对应的buffer
        */
        // av_packet_unref(pkt);
    }
    return -1;
}

static void f32le_convert_to_fltp(const float *f32le, float *fltp,const int &nb_samples) {
    float *fltp_l = fltp;   // 左通道
    float *fltp_r = fltp + nb_samples;   // 右通道
    for(int i {}; i < nb_samples; i++) {
        fltp_l[i] = f32le[i * 2];     // 0 1   - 2 3
        fltp_r[i] = f32le[i*2 + 1];   // 可以尝试注释左声道或者右声道听听声音
    }
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
    const F fn;
};

int main(const int argc,const char* argv[])
{
    if (argc < 3){
        std::cerr << "Usage: " << argv[0] << " <input_file out_file [codec_name]>, argc:" << argc << "\n";
        return -1;
    }

    std::pmr::unsynchronized_pool_resource mptool;
    std::ifstream in_file(argv[1],std::ios::binary);
    std::ofstream out_file(argv[2],std::ios::binary);
    constexpr auto codec_id {AV_CODEC_ID_AAC};
    const AVCodec * codec{};
    AVCodecContext *codec_ctx{};
    uint8_t* pcm_buf{},*pcm_tmp_buf{};
    std::size_t pcm_buf_size{};

    bool force_{};
    std::string code_name;
    if (4 == argc){
        if (std::string("libfdk_aac") == argv[3] || std::string("aac") == argv[3]){
            force_ = true;
            code_name = argv[3];
        }else{
            code_name = "aac";
        }
    }

    const Destroyer d([&](){
        if (pcm_tmp_buf){
            mptool.deallocate(pcm_tmp_buf,pcm_buf_size);
        }
        if (pcm_buf){
            mptool.deallocate(pcm_buf,pcm_buf_size);
        }

        avcodec_free_context(&codec_ctx);
        out_file.close();
        in_file.close();
    });

    if (!in_file){
        std::cerr << "open in_file failed\n";
        return -1;
    }

    if (!out_file){
        std::cerr << "open out_file failed\n";
        return -1;
    }

    if (force_){
        std::cout << "force codec name: " << code_name << "\n";
        codec = avcodec_find_encoder_by_name(code_name.c_str());
    }else{
        codec = avcodec_find_encoder(codec_id);
        std::cout << "default codec name: aac \n";
    }

    if (!codec){
        std::cerr << "Codec not found\n";
        return -1;
    }

    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx){
        std::cerr << "avcodec_alloc_context3 failed\n";
        return -1;
    }

    codec_ctx->codec_id = codec_id;
    codec_ctx->codec_type = AVMEDIA_TYPE_AUDIO;
    //codec_ctx->bit_rate = 128*1024;
    codec_ctx->sample_rate = 48000;
    codec_ctx->ch_layout = AV_CHANNEL_LAYOUT_STEREO;
    codec_ctx->profile = FF_PROFILE_AAC_LOW;

    if(std::string("libfdk_aac") == codec->name) {
        codec_ctx->sample_fmt = AV_SAMPLE_FMT_S16;
    } else {
        codec_ctx->sample_fmt = AV_SAMPLE_FMT_FLTP;
    }

    if (!check_channel_layout(codec,codec_ctx->ch_layout)){
        std::cerr << "Encoder does not support channel layout " << codec_ctx->ch_layout.u.mask << "\n";
        return -1;
    }

    if (!check_sample_fmt(codec,codec_ctx->sample_fmt)){
        std::cerr << "Encoder does not support sample format " <<
            av_get_sample_fmt_name(codec_ctx->sample_fmt) << "\n";
        return -1;
    }

    if (!check_sample_rate(codec,codec_ctx->sample_rate)){
        std::cerr << "Encoder does not support sample rate " << codec_ctx->sample_rate << "\n";
        return -1;
    }

    codec_ctx->flags = AV_CODEC_FLAG_GLOBAL_HEADER;  //ffmpeg默认的aac是不带adts，而fdk_aac默认带adts，这里我们强制不带

    if (avcodec_open2(codec_ctx,codec,nullptr) < 0){
        std::cerr << "avcodec_open2 failed\n";
        return -1;
    }




    return 0;
}
