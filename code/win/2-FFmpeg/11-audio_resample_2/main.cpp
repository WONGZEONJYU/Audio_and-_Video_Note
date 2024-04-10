#include <iostream>
#include <fstream>

#include "Audio_Resampler.h"

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

static int get_format_from_sample_fmt(const char **fmt,
                                      const AVSampleFormat &sample_fmt)
{
    struct sample_fmt_entry {
        AVSampleFormat sample_fmt; const char *fmt_be, *fmt_le;
    } constexpr sample_fmt_entries[] = {
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

    std::cerr << "Sample format " << av_get_sample_fmt_name(sample_fmt) << " not supported as output format\n";
    return AVERROR(EINVAL);
}

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

int main(const int argc,const char* argv[])
{
     constexpr rsmp::Audio_Resampler_Params params {
         /*input parms*/
        .src_sample_fmt = AV_SAMPLE_FMT_DBL,
        .src_ch_layout = AV_CHANNEL_LAYOUT_STEREO,
        .src_sample_rate = 48000,
         /*output parms*/
        .dst_sample_fmt =AV_SAMPLE_FMT_S16,
        .dst_ch_layout = AV_CHANNEL_LAYOUT_STEREO,
        .dst_sample_rate = 44100,
    };

    rsmp::Audio_Resampler::Audio_Resampler_t resampler;

     try {
        resampler = rsmp::Audio_Resampler::create(params);

     } catch (std::exception &e) {
         std::cerr << e.what() << "\n";
         return -1;
     }

    const auto src_nb_samples {1024};
    int src_linesize{};
    uint8_t **src_data{};

    const auto dst_nb_samples {1152};
    int dst_linesize{};
    uint8_t** dst_data{};

    std::ofstream out_file(argv[1] , std::ios::binary);
     if (!out_file) {
         std::cerr << "open out_file failed\n";
         return -1;
     }

    Destroyer d([&]()
    {
        out_file.close();
        if (src_data){
            av_freep(&src_data[0]);
            av_freep(&src_data);
        }

        if (dst_data){
            av_freep(&dst_data[0]);
            av_freep(&dst_data);
        }

    });

    /*输入源分配空间*/
    constexpr auto src_nb_channels{params.src_ch_layout.nb_channels};
     if (av_samples_alloc_array_and_samples(&src_data,
         &src_linesize,
         src_nb_channels,
         src_nb_samples,
         params.src_sample_fmt,0) < 0){

         std::cerr << "Could not allocate source samples\n";
         return -1;
     }

    /*分配输出缓存*/
    constexpr auto dst_nb_channels{params.dst_ch_layout.nb_channels};
     if (av_samples_alloc_array_and_samples(&dst_data,
         &dst_linesize,
         dst_nb_channels,
         dst_nb_samples,
         params.dst_sample_fmt,0) < 0){
         std::cerr << "Could not allocate destination samples\n";
         return -1;
     }

    int64_t in_pts{},out_pts{};
    double t{};

     do{
         fill_samples(reinterpret_cast<double*>(src_data[0]),src_nb_samples,
             src_nb_channels, params.src_sample_rate, t);

         const auto send_size{resampler->send_frame(src_data,src_nb_samples,in_pts)};

         if (send_size < 0){
             return -1;
         }

         in_pts += src_nb_samples;

         const auto receive_size{resampler->receive_frame(dst_data,dst_nb_samples,out_pts)};

         if (receive_size > 0){

             const auto dst_bufsize {av_samples_get_buffer_size(&dst_linesize, dst_nb_channels,
                                                      receive_size, params.dst_sample_fmt, 1)};

             if (dst_bufsize < 0){
                 std::cerr << "Could not get sample buffer size\n";
                 return -1;
             }

             std::cout << "dst_bufsize = " << dst_bufsize <<
                 " ,dst_linesize = " << dst_linesize << "\n" << std::flush;

             std::cout << "t: " << t << " in: " << src_nb_samples << " out: " << receive_size <<
                 ", out_pts: " << out_pts << "\n" << std::flush;

            out_file.write(reinterpret_cast<const char*>(dst_data[0]),dst_bufsize);
         }else{
             std::cout << "can't get " << dst_nb_samples <<
                 " samples, receive_size : " << receive_size << ", cur_size : " <<
                     resampler->fifo_size() << "\n" << std::flush;
         }

     }while (t < 10);

    /*把剩余的数据读取出来*/
    resampler->send_frame(static_cast<uint8_t**>(nullptr),0,0);
    const auto fifo_size{resampler->fifo_size()};
    const auto get_size{fifo_size > dst_nb_samples ? dst_nb_samples : fifo_size};
    const auto receivce_size{resampler->receive_frame(dst_data,get_size,out_pts)};

    if (receivce_size > 0){ /*av_samples_set_silence用于填充音频缓冲区*/
        av_samples_set_silence(dst_data, receivce_size, dst_nb_samples - receivce_size,
            dst_nb_channels, params.dst_sample_fmt); /*用静音填充*/
        const auto dst_bufsize {av_samples_get_buffer_size(&dst_linesize, dst_nb_channels,
                                                      dst_nb_samples, params.dst_sample_fmt, 1)};

        if (dst_bufsize < 0){
            std::cerr << "Could not get sample buffer size\n";
            return -1;
        }
        std::cout << "flush in: " << receivce_size << ", out_pts: " << out_pts << "\n" << std::flush;
        out_file.write(reinterpret_cast<const char*>(dst_data[0]),dst_bufsize);
     }

    const char *fmt{};
    if (get_format_from_sample_fmt(&fmt, params.dst_sample_fmt) < 0){
        return -1;
    }

    std::cerr << "Resampling succeeded. Play the output file with the command:\n" <<
        "ffplay -f " << fmt << " -channel_layout " << params.dst_ch_layout.u.mask <<
            " -channels " << dst_nb_channels<< " -ar " << params.dst_sample_rate <<
            " " << argv[1] << "\n";

    return 0;
}
