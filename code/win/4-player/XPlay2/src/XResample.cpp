//
// Created by Administrator on 2024/8/2.
//
extern "C"{
#include <libavutil/samplefmt.h>
}
#include <iostream>

#include "XResample.hpp"
#include "XSwrContext.hpp"
#include "XAVCodecParameters.hpp"

void XResample::Open(const XAVCodecParameters_sptr &parm) {

    if (!parm){
        std::cerr << __func__ << " parm is empty";
        return;
    }

    std::unique_lock lock(m_mux);
    m_swr_ctx.reset();
    CHECK_EXC(m_swr_ctx = new_XSwrContext(parm->ch_layout(),
                                          AV_SAMPLE_FMT_S16,
                                          parm->sample_rate(),
                                          parm->ch_layout(),
                                          static_cast<AVSampleFormat>(parm->sampleFormat()),
                                          parm->sample_rate()),lock.unlock());
}

int XResample::Resample(const XAVFrame_sptr &frame,std::vector<uint8_t> &data) noexcept(false) {

    if (!frame){
        std::cerr << __func__ << "XAVFrame_sptr is empty\n";
        return -1;
    }

    std::unique_lock lock(m_mux);

    if (!m_swr_ctx){
        std::cerr <<  __func__ << "m_swr_ctx is empty\n";
        return -1;
    }

    auto ret{-1};
    FF_CHECK_ERR(ret = m_swr_ctx->convert());

    return ret;
}

