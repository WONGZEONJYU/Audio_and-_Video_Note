#ifndef AUDIO_RESAMPLER_H
#define AUDIO_RESAMPLER_H

#include <memory>

extern "C" {
#include <libavutil/audio_fifo.h>
#include <libavutil/fifo.h>
#include <libavutil/opt.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavcodec/avcodec.h>
}

namespace rsmp
{
    struct Audio_Resampler_Params {
        // input params
        AVSampleFormat src_sample_fmt;
        AVChannelLayout src_ch_layout;
        int src_sample_rate;
        uint64_t src_channel_layout;

        // output params
        AVSampleFormat dst_sample_fmt;
        AVChannelLayout dst_ch_layout;
        int dst_sample_rate;
        uint64_t dst_channel_layout;
    };

    class Audio_Resampler {

    public:
        Audio_Resampler(const Audio_Resampler&) = delete;
        Audio_Resampler& operator=(const Audio_Resampler&) = delete;
        explicit Audio_Resampler() = default;
        explicit Audio_Resampler(Audio_Resampler_Params);


    private:
        Audio_Resampler_Params Resampler_Params{};


    };
}





#endif //AUDIO_RESAMPLER_H
