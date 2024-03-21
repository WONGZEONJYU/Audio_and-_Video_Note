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
    bool Audio_Resampler::init() {
        return {};
    }

    Audio_Resampler::Audio_Resampler(const Audio_Resampler_Params & params) {

        m_Resampler_Params = params;


    }






}
