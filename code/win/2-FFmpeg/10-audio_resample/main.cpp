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
    F fn;
};

int main(const int argc,const char* argv[]) {

    if (argc < 2) {
        std::cerr << "Usage: " << argv[1] <<  " output_file\n";
        return -1;
    }

    std::ofstream dst_filename(argv[1]);

    const auto rres{[&]() {
        if (dst_filename) {
            dst_filename.close();
        }
    }};

    /*
     * 输入参数
     */
    
    return 0;
}
