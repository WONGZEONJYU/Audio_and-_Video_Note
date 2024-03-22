//
// Created by Administrator on 2024/3/21.
//

extern "C"{
#include <libavutil/fifo.h>
#include <libavutil/opt.h>
#include <libavutil/avutil.h>
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavcodec/avcodec.h>
}

#include "Audio_Resampler.h"

namespace rsmp {

     const char * Audio_Resampler::AVAudioFifo_exp:: what() const noexcept{
         return "av_audio_fifo_alloc faild\n";
     }

    bool Audio_Resampler::init(const Audio_Resampler_Params &params) {

        m_audio_fifo = std::move(std::make_shared<AVAudioFifo_t>(params.dst_sample_fmt,
            m_dst_channels,1));

        if (!m_audio_fifo){
            throw AVAudioFifo_exp();
        }

        m_Resampler_Params = params;
        m_src_channels = params.src_ch_layout.u.mask;
        m_dst_channels = params.dst_ch_layout.u.mask;

        if (params){    /*参数相同就不需要做重采样*/
            m_is_fifo_only = true;
            return true;
        }

        return {};
    }

    Audio_Resampler::Audio_Resampler(const Audio_Resampler_Params & params) {
        init(params);
    }

    Audio_Resampler::~Audio_Resampler(){
         av_freep(&m_resampled_data);
    }
}
