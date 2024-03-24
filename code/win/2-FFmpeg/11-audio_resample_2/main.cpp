#include <iostream>
#include <fstream>

#include "Audio_Resampler.h"

int main(const int argc,const char* argv[])
{
     constexpr rsmp::Audio_Resampler_Params params {
        .src_sample_fmt = AV_SAMPLE_FMT_DBL,
        .src_ch_layout = AV_CHANNEL_LAYOUT_STEREO,
        .src_sample_rate = 1024,
        .dst_sample_fmt =AV_SAMPLE_FMT_S16,
        .dst_ch_layout = AV_CHANNEL_LAYOUT_STEREO,
        .dst_sample_rate = 1152,
    };

     try {
         auto re{rsmp::Audio_Resampler::create(params)};
     } catch (std::exception &e) {
         std::cerr << e.what() << "\n";
     }

    return 0;
}
