#include <iostream>
#include <string>
#include <fstream>
#include <memory_resource>

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
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
    constexpr auto ERROR_STRING_SIZE{1024};
    char err_buf[ERROR_STRING_SIZE]{};
    av_strerror(errnum, err_buf, sizeof(err_buf));
    return {err_buf};
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
    AVPacket pkt{};

    auto rres{[&]() {
        out_h264_file.close();
        out_aac_file.close();
        av_packet_unref(&pkt);
        avformat_close_input(&ifmt_ctx);
        avformat_free_context(ifmt_ctx);
    }};

    Destroyer d(std::move(rres));

    ifmt_ctx = avformat_alloc_context();
    if (!ifmt_ctx) {
        std::cerr << "avformat_alloc_context failed\n";
        return -1;
    }

    auto ret{avformat_open_input(&ifmt_ctx,in_file,nullptr,nullptr)};
    if (ret < 0) {
        std::cerr << av_get_err(ret);
        return -1;
    }

    return 0;
}
