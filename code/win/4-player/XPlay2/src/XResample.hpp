//
// Created by Administrator on 2024/8/2.
//

#ifndef XPLAY2_XRESAMPLE_HPP
#define XPLAY2_XRESAMPLE_HPP

#include <memory>
#include <vector>
#include <mutex>
#include "XHelper.hpp"

class XSwrContext;
class XAVCodecParameters;
class XAVFrame;
using resample_data_t = typename std::vector<uint8_t>;

class XResample {
    using SwrContext_sptr = typename std::shared_ptr<XSwrContext>;
    using XAVCodecParameters_sptr = typename std::shared_ptr<XAVCodecParameters>;
    using XAVFrame_sptr = typename std::shared_ptr<XAVFrame>;

public:
    explicit XResample() = default;
    virtual void Open(const XAVCodecParameters_sptr &) noexcept(false);
    virtual void Close() noexcept(true);

    /**
     * 重采样,out_samples用于返回重采样后的sample的个数
     * @param frame
     * @param datum
     * @param out_samples
     * @return (out_samples * channels * bytes_per_sample)
     */
    virtual int Resample(const XAVFrame_sptr &frame,
                         resample_data_t &datum,
                         int &out_samples) noexcept(false);

protected:
    std::mutex m_mux;
    SwrContext_sptr m_swr_ctx;

public:
    virtual ~XResample() = default;
    X_DISABLE_COPY(XResample)
};

#endif
