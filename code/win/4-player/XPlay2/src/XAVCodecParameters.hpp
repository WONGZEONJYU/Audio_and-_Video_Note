//
// Created by Administrator on 2024/7/30.
//

#ifndef XPLAY2_XAVCODECPARAMETERS_HPP
#define XPLAY2_XAVCODECPARAMETERS_HPP

#include <memory>
#include "XHelper.h"

struct AVCodecParameters;
struct AVCodecContext;
class XAVCodecParameters;

using XAVCodecParameters_sptr = std::shared_ptr<XAVCodecParameters>;

class XAVCodecParameters final {
    friend XAVCodecParameters_sptr new_XAVCodecParameters() noexcept(false);
    explicit XAVCodecParameters() = default;
    void Construct() noexcept(false);
    void DeConstruct() noexcept(true);
public:
    void from_AVFormatContext(const AVCodecParameters *) const noexcept(false);
    void from_context(const AVCodecContext *) const noexcept(false);
    void to_context(AVCodecContext *) const noexcept(false);
private:
    AVCodecParameters *m_parm{};
public:
    ~XAVCodecParameters();
    X_DISABLE_COPY_MOVE(XAVCodecParameters)
};

XAVCodecParameters_sptr new_XAVCodecParameters() noexcept(false);

#endif //XPLAY2_XAVCODECPARAMETERS_HPP
