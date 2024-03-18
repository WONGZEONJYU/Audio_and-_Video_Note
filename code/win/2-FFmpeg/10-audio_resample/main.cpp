#include <iostream>
#include <fstream>
#include <stdnoreturn.h>

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
        std::cerr << __FUNCTION__ << "\n";
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

int main(const int argc,const char* argv[]) {

    // if (argc < 2) {
    //     std::cerr << "Usage: " << argv[1] <<  " output_file\n";
    //     return -1;
    // }
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
    int dst_nb_samples{};
    int max_dst_nb_samples{};

    SwrContext *swr_ctx{};

    const auto rres{[&]() {
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
    }};

    Destroyer d{std::move(rres)};

    // 创建重采样器
    /* create resampler context */
    if (!(swr_ctx = swr_alloc())) {
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
    ret = av_samples_alloc_array_and_samples(&src_data,&src_linesize,src_nb_channels,src_nb_samples,src_sample_fmt,0);
    if (ret < 0) {
        std::cerr << "Could not allocate source samples\n";
        return -1;
    }


    return 0;
}
