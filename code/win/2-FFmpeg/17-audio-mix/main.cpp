#include <iostream>
#include <sstream>
#include "Audio_Mix.hpp"

int main(const int argc,const char **argv) {

    if(argc < 4){
        std::cerr << "usage in1.pcm in2.pcm out.pcm";
        return -1;
    }

    try {
        auto mix {new_Audio_Mix(argv[1],argv[2],argv[3])};

        AudioInfo info0("input(0)",48000,AV_SAMPLE_FMT_FLTP,AV_CHANNEL_LAYOUT_STEREO);
        AudioInfo info1("input(1)",48000,AV_SAMPLE_FMT_S16,AV_CHANNEL_LAYOUT_STEREO);
        AudioInfo out_info("out",96000,AV_SAMPLE_FMT_S16,AV_CHANNEL_LAYOUT_STEREO);

        mix->push_in_audio_info(0,std::move(info0));
        mix->push_in_audio_info(1,std::move(info1));

        mix->push_out_audio_info(std::move(out_info));

        mix->init();

    } catch (const std::exception &e) {
        std::cerr << e.what() << "\n";
        return -1;
    }

    return 0;
}
