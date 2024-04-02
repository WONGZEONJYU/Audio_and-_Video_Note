
extern "C"{
#include <libavcodec/avcodec.h>
//#include <libavcodec/codec.h>
//#include <libavcodec/codec_id.h>
#include <libavformat/avformat.h>
}

#include <iostream>
#include <fstream>
#include <memory_resource>

static std::string av_get_err(const int& errnum)
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
    int freq_idx ;    //0: 96000 Hz  3: 48000 Hz 4: 44100 Hz
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

    auto ret {frame.extended_data ? avcodec_send_frame(&ctx, &frame) : avcodec_send_frame(&ctx, nullptr)};

    if (ret < 0) {
        std::cout << "Error sending the frame to the encoder\n";
        return -1;
    }

    /* read all the available output packets (in general there may be any number of them */
    // 编码和解码都是一样的,都是send 1次,然后receive多次, 直到AVERROR(EAGAIN)或者AVERROR_EOF
    for(;;) {
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
    //return -1;
}

static void f32le_convert_to_fltp(const float *f32le, float *fltp,const int &nb_samples) {
    float *fltp_l = fltp;   // 左通道
    float *fltp_r = fltp + nb_samples;   // 右通道
    for(int i {}; i < nb_samples; i++) {
        fltp_l[i] = f32le[i * 2];     // 0 1   - 2 3
        fltp_r[i] = f32le[i * 2 + 1];   // 可以尝试注释左声道或者右声道听听声音
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

/*xxx.exe xxx_FMT_S16.pcm xxx.aac libfdk_aac*/
/*xxx.exe xxx_FMT_FLTP.pcm xxx.aac aac*/
/*libfdk_aac只支持S16*/

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
    AVPacket *pkt{};
    AVFrame *frame{};
    uint8_t* pcm_buf{},*pcm_tmp_buf{};
    std::size_t one_frame_size{};

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
            mptool.deallocate(pcm_tmp_buf,one_frame_size);
        }
        if (pcm_buf){
            mptool.deallocate(pcm_buf,one_frame_size);
        }
        av_frame_free(&frame);
        av_packet_free(&pkt);
        avcodec_free_context(&codec_ctx);
        out_file.close();
        in_file.close();
        std::cerr << "finish\n";
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

    std::cout << "frame size = " << codec_ctx->frame_size << "\n";

    if (avcodec_open2(codec_ctx,codec,nullptr) < 0){
        std::cerr << "avcodec_open2 failed\n";
        return -1;
    }

    std::cout << "frame size = " << codec_ctx->frame_size << "\n";
    std::cout << "\n\nAudio encode config\n";
    std::cout << "channels : " << codec_ctx->ch_layout.nb_channels << "\n";
    std::cout << "sample_rate : " << codec_ctx->sample_rate << "\n";
    std::cout << "sample_format : " << codec_ctx->sample_fmt << "\n";

    pkt = av_packet_alloc();
    if (!pkt) {
        std::cerr << "av_packet_alloc failed\n";
        return -1;
    }

    frame = av_frame_alloc();
    if (!frame) {
        std::cerr << "av_frame_alloc failed\n";
        return -1;
    }

    frame->nb_samples = codec_ctx->frame_size;
    frame->format = codec_ctx->sample_fmt;
    frame->ch_layout = codec_ctx->ch_layout;

    if (av_frame_get_buffer(frame,0) < 0) {
        std::cerr << "Could not allocate audio data buffers\n";
        return -1;
    }
    /*calc one_frame_size*/
    const auto format_size{av_get_bytes_per_sample(static_cast<AVSampleFormat>(frame->format))};
    one_frame_size = frame->nb_samples * format_size * frame->ch_layout.nb_channels;

    try {
        pcm_buf = static_cast<uint8_t *>(mptool.allocate(one_frame_size));
    } catch (std::exception &e) {
        std::cout << "alloc pcm_buf failed : " << e.what() << "\n";
        return -1;
    }

    const auto is_FMT_S16{AV_SAMPLE_FMT_S16 == frame->format};

    if (!is_FMT_S16) {
        try {
            pcm_tmp_buf = static_cast<uint8_t *>(mptool.allocate(one_frame_size));
        } catch (std::exception &e) {
            std::cout << "alloc pcm_buf failed : " << e.what() << "\n";
            return -1;
        }
    }

    int64_t pts {};
    std::cout << "start enode\n";

    for(;;) {
        std::fill_n(pcm_buf,one_frame_size,0);
        try {
            in_file.read(reinterpret_cast<char *>(pcm_buf),static_cast<std::streamsize>(one_frame_size));
            if (in_file.eof()) {
                std::cerr << "read finish\n";
                break;
            }
        } catch (std::exception &e) {
            std::cerr << "read failed " << e.what() << "\n";
            return -1;
        }

        /* 确保该frame可写,如果编码器内部保持了内存参考计数,则需要重新拷贝一个备份
            目的是新写入的数据和编码器保存的数据不能产生冲突*/
        if (av_frame_make_writable(frame) < 0) {
            std::cerr << "av_frame_make_writable\n";
        }

        if (is_FMT_S16) {
            // 将读取到的PCM数据填充到frame去，但要注意格式的匹配, 是planar还是packed都要区分清楚
            if(av_samples_fill_arrays(frame->data,frame->linesize,pcm_buf,
                   frame->ch_layout.nb_channels,
                   frame->nb_samples,static_cast<AVSampleFormat>(frame->format),0) < 0) {
                std::cerr << "1 av_samples_fill_arrays failed\n";
                return -1;
            }

        }else {
            // 将本地的f32le packed模式的数据转为float palanar
            std::fill_n(pcm_tmp_buf,one_frame_size,0);
            f32le_convert_to_fltp(reinterpret_cast<const float *>(pcm_buf),reinterpret_cast<float *>(pcm_tmp_buf),frame->nb_samples);
            if(av_samples_fill_arrays(frame->data,frame->linesize,pcm_tmp_buf,
                            frame->ch_layout.nb_channels,
                            frame->nb_samples,static_cast<AVSampleFormat>(frame->format),0) < 0) {
                std::cerr << "2 av_samples_fill_arrays failed\n";
                return -1;
            }
        }

        // 设置pts
        pts += frame->nb_samples;
        frame->pts = pts;       // 使用采样率作为pts的单位，具体换算成秒 pts*1/采样率
        if (encode(*codec_ctx,*frame,*pkt,out_file) < 0) {
            std::cerr << "encode failed\n";
            break;
        }
    }

    encode(*codec_ctx, {}, *pkt, out_file);
    return 0;
}
