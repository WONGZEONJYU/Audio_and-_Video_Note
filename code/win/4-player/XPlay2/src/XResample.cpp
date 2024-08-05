//
// Created by Administrator on 2024/8/2.
//
extern "C"{
#include <libavutil/samplefmt.h>
}

#include "XResample.hpp"
#include "XSwrContext.hpp"
#include "XAVCodecParameters.hpp"
#include "XAVFrame.hpp"
#include <iostream>

void XResample::Open(const XAVCodecParameters_sptr &parm) {

    if (!parm){
        PRINT_ERR_TIPS(XAVCodecParameters_sptr is empty);
        return;
    }

    Close();
    std::unique_lock lock(m_mux);
    CHECK_EXC(m_swr_ctx = new_XSwrContext(parm->Ch_layout(),
                                          AV_SAMPLE_FMT_S16,
                                          parm->Sample_rate(),
                                          parm->Ch_layout(),
                                          static_cast<AVSampleFormat>(parm->Sample_Format()),
                                          parm->Sample_rate()),lock.unlock());
}

void XResample::Close() noexcept(true) {
    std::unique_lock lock(m_mux);
    m_swr_ctx.reset();
}

int XResample::Resample(const XAVFrame_sptr &frame,resample_data_t &datum) noexcept(false) {

    if (!frame){
        PRINT_ERR_TIPS(XAVFrame_sptr is empty);
        return -1;
    }

    std::unique_lock lock(m_mux);

    if (!m_swr_ctx){
        PRINT_ERR_TIPS(Please initialize first);
        return -1;
    }

    //+ 256的目的是重采样内部是有一定的缓存,就存在上一次的重采样缓存数据和这一次重采样一起输出的情况,多出来的目的是为了分配大点的输出buffer
    const auto out_count {frame->nb_samples * frame->sample_rate / frame->sample_rate + 256};
    const auto out_size{av_samples_get_buffer_size(nullptr,frame->ch_layout.nb_channels,out_count,
                                 AV_SAMPLE_FMT_S16,1)};

    if (datum.capacity() < out_size){
        datum.resize(out_size);
    }

    uint8_t *d[AV_NUM_DATA_POINTERS]{datum.data()};

    auto ret_nb_samples{-1};
    FF_CHECK_ERR(ret_nb_samples = m_swr_ctx->convert(d,out_count,frame->data,frame->nb_samples),lock.unlock());

    return av_samples_get_buffer_size(nullptr,frame->ch_layout.nb_channels,ret_nb_samples,AV_SAMPLE_FMT_S16,1);
}

