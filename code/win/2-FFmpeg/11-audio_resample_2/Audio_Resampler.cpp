#include <iostream>

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

    bool Audio_Resampler::construct() {

        m_src_channels = m_Resampler_Params.src_ch_layout.u.mask;
        m_dst_channels = m_Resampler_Params.dst_ch_layout.u.mask;

        m_audio_fifo = std::move(std::make_shared<AVAudioFifo_t>
        (m_Resampler_Params.dst_sample_fmt,
        m_dst_channels,1));

        if (!m_audio_fifo) {

            return {};
        }

        if (m_Resampler_Params) {
            m_is_fifo_only.store(true);
            return true;
        }

        m_swr_ctx = std::move(std::make_shared<SwrContext_t>());
        if (!m_swr_ctx) {

            return {};
        }


        return true;
    }

    Audio_Resampler::Audio_Resampler(const Audio_Resampler_Params & params) {
        m_Resampler_Params = params;
    }

    Audio_Resampler::~Audio_Resampler(){
         av_freep(&m_resampled_data);
    }
}
