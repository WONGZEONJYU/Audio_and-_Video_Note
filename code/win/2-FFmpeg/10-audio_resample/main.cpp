#include <iostream>
#include <fstream>

extern "C"
{
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
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

static std::string av_get_err(const int& errnum)
{
    constexpr auto ERROR_STRING_SIZE {1024};
    char err_buf[ERROR_STRING_SIZE]{};
    av_strerror(errnum, err_buf, std::size(err_buf));
    return {err_buf};
}

static int get_format_from_sample_fmt(const char **fmt,
                                      const AVSampleFormat &sample_fmt)
{

    struct sample_fmt_entry {
        AVSampleFormat sample_fmt; const char *fmt_be, *fmt_le;
    } sample_fmt_entries[] = {
        { AV_SAMPLE_FMT_U8,  "u8",    "u8"    },
        { AV_SAMPLE_FMT_S16, "s16be", "s16le" },
        { AV_SAMPLE_FMT_S32, "s32be", "s32le" },
        { AV_SAMPLE_FMT_FLT, "f32be", "f32le" },
        { AV_SAMPLE_FMT_DBL, "f64be", "f64le" },
    };

    for (const auto &entry : sample_fmt_entries){
        if (sample_fmt == entry.sample_fmt) {
            *fmt = AV_NE(entry.fmt_be, entry.fmt_le);
            return 0;
        }
    }

    std::cerr << "Sample format :" << av_get_sample_fmt_name(sample_fmt) <<
        " not supported as output format\n";
    return AVERROR(EINVAL);
}


/**
 * Fill dst buffer with nb_samples, generated starting from t. 交错模式的
 */
static void fill_samples(double *dst,const int &nb_samples,const int &nb_channels, const int &sample_rate, double &t)
{
    const auto tincr {1.0 / sample_rate};
    constexpr auto c {2 * M_PI * 440.0};

    for (int i{};i < nb_samples;++i){
        dst[0] = sin(c * t);
        for(int j{1};j < nb_channels;++j){
            dst[j] = dst[0];
        }
        dst += nb_channels;
        t += tincr;
    }
}

int main(const int argc,const char* argv[]) {

    if (argc < 2) {
        std::cerr << "Usage: " << argv[1] <<  " output_file\n";
        return -1;
    }

    std::ofstream dst_filename(argv[1]);
    /*
     * 输入参数
     */
    //constexpr auto src_ch_layout{AV_CH_LAYOUT_STEREO};
    constexpr AVChannelLayout src_ch_layout AV_CHANNEL_LAYOUT_STEREO;
    constexpr auto src_rate {48000};
    constexpr auto src_sample_fmt {AV_SAMPLE_FMT_DBL};
    constexpr auto src_nb_channels{src_ch_layout.nb_channels};
    uint8_t **src_data{};
    int src_linesize{};
    constexpr auto src_nb_samples {1024};

    /*
     * 输出参数
     */
    //constexpr auto dst_ch_layout {AV_CH_LAYOUT_STEREO};
    constexpr AVChannelLayout dst_ch_layout AV_CHANNEL_LAYOUT_STEREO;
    constexpr auto dst_rate {44100};
    constexpr auto dst_sample_fmt {AV_SAMPLE_FMT_S16};
    constexpr auto dst_nb_channels {dst_ch_layout.nb_channels};
    uint8_t **dst_data {};
    int dst_linesize{};
    int64_t dst_nb_samples{},max_dst_nb_samples{};

    SwrContext *swr_ctx{};

    Destroyer d{std::move([&]() {
        if (dst_filename) {
            dst_filename.close();
        }
        if (src_data) {
            av_freep(&src_data[0]);
            av_freep(&src_data);
        }
        if (dst_data) {
            av_freep(&dst_data[0]);
            av_freep(&dst_data);
        }
        swr_free(&swr_ctx);
        std::cerr << "destory complete\n";
    })};

    // 创建重采样器
    /* create resampler context */
    swr_ctx = swr_alloc();
    if (!(swr_ctx)) {
        std::cerr << "swr_alloc faild\n";
        return -1;
    }

    // 设置重采样参数
    /* set options */
    // 输入参数
    av_opt_set_chlayout(swr_ctx,"in_chlayout",&src_ch_layout,0);
    av_opt_set_int(swr_ctx, "in_sample_rate",src_rate, 0);
    av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt",src_sample_fmt, 0);
    // 输出参数
    av_opt_set_chlayout(swr_ctx, "out_chlayout",&dst_ch_layout, 0);
    av_opt_set_int(swr_ctx, "out_sample_rate",dst_rate, 0);
    av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", dst_sample_fmt, 0);

   // 初始化重采样
   // /* initialize the resampling context */
    auto ret{swr_init(swr_ctx)};
    if (ret < 0) {
        std::cerr << "Failed to initialize the resampling context : " << av_get_err(ret) << "\n";
        return -1;
    }
    // 给输入源分配内存空间
    ret = av_samples_alloc_array_and_samples(&src_data,&src_linesize,src_nb_channels,src_nb_samples,src_sample_fmt,0);
    if (ret < 0) {
        std::cerr << "Could not allocate source samples\n";
        return -1;
    }

    /* compute the number of converted samples: buffering is avoided
    * ensuring that the output buffer will contain at least all the
    * converted input samples
    */
    // 计算输出采样数量
    max_dst_nb_samples = dst_nb_samples =
        av_rescale_rnd(src_nb_samples, dst_rate, src_rate, AV_ROUND_UP);
    // 分配输出缓存内存
    ret = av_samples_alloc_array_and_samples(&dst_data,&dst_linesize,dst_nb_channels,static_cast<int>(dst_nb_samples),dst_sample_fmt,0);
    if (ret < 0){
        std::cerr << "Could not allocate destination samples\n";
        return -1;
    }

    double t{};
    int dst_bufsize{};
    do{
        fill_samples(reinterpret_cast<double*>(src_data[0]), src_nb_samples, src_nb_channels, src_rate, t);
        /* compute destination number of samples */
        const auto delay{swr_get_delay(swr_ctx, src_rate)};
        dst_nb_samples = av_rescale_rnd(delay + src_nb_samples, dst_rate, src_rate, AV_ROUND_UP);

        if (dst_nb_samples > max_dst_nb_samples){
            av_freep(&dst_data[0]);
            ret = av_samples_alloc(dst_data, &dst_linesize, dst_nb_channels,static_cast<int>(dst_nb_samples), dst_sample_fmt, 1);
            if (ret < 0){
                std::cerr << "av_samples_alloc failed : " << av_get_err(ret) << "\n";
                break;
            }

            max_dst_nb_samples = dst_nb_samples;
        }

        const auto ret_len { swr_convert(swr_ctx, dst_data, static_cast<int>(dst_nb_samples),
                          const_cast<const uint8_t**>(src_data), src_nb_samples)};
        if (ret_len < 0){
            std::cerr << "Error while converting\n";
            return -1;
        }

        dst_bufsize = av_samples_get_buffer_size(&dst_linesize, dst_nb_channels,ret_len, dst_sample_fmt, 1);

        if (dst_bufsize < 0) {
            std::cerr << "Could not get sample buffer size\n";
            return -1;
        }

        std::cout << "t: " << t << " in: " << src_nb_samples << " out: " << ret_len << "\n";
        dst_filename.write(reinterpret_cast<const std::ostream::char_type*>(dst_data[0]),dst_bufsize);

    }while (t < 10);

    ret = swr_convert(swr_ctx, dst_data, static_cast<int>(dst_nb_samples), nullptr, 0);
    if (ret < 0) {
        fprintf(stderr, "Error while converting\n");
        return -1;
    }

    dst_bufsize = av_samples_get_buffer_size(&dst_linesize, dst_nb_channels,
                                             ret, dst_sample_fmt, 1);
    if (dst_bufsize < 0) {
        fprintf(stderr, "Could not get sample buffer size\n");
        return -1;
    }
    
    std::cout << "flush in: 0 out: " << ret << "\n";
    
    dst_filename.write(reinterpret_cast<const std::ostream::char_type*>(dst_data[0]),dst_bufsize);

    const char *fmt{};
    if ((ret = get_format_from_sample_fmt(&fmt, dst_sample_fmt)) < 0){
        return -1;
    }

    std::cerr << "Resampling succeeded. Play the output file with the command:\n" <<
        "ffplay -f " << fmt << " -channel_layout " << dst_ch_layout.u.mask << " -channels " << dst_nb_channels << " -ar " << dst_rate <<
            " " << argv[1] << "\n";

    return 0;
}
