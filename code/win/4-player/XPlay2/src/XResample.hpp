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

using SwrContext_sptr = std::shared_ptr<XSwrContext>;
using XAVCodecParameters_sptr = typename std::shared_ptr<XAVCodecParameters>;
using XAVFrame_sptr = std::shared_ptr<XAVFrame>;

using resample_data_t = std::vector<uint8_t>;

class XResample {

public:
    explicit XResample() = default;
    virtual void Open(const XAVCodecParameters_sptr &) noexcept(false);
    virtual void Close() noexcept(true);
    virtual int Resample(const XAVFrame_sptr &,resample_data_t &) noexcept(false);

protected:
    std::mutex m_mux;
    SwrContext_sptr m_swr_ctx;

public:
    virtual ~XResample() = default;
    X_DISABLE_COPY(XResample)
};

#endif
