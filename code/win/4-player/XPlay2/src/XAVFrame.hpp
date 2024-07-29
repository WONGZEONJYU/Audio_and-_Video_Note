//
// Created by Administrator on 2024/7/29.
//

#ifndef XPLAY2_XAVFRAME_HPP
#define XPLAY2_XAVFRAME_HPP

extern "C"{
#include <libavutil/frame.h>
};

#include <memory>

struct XAVFrame final : public AVFrame {

    XAVFrame();
    XAVFrame(const XAVFrame &);
    XAVFrame(XAVFrame &&) noexcept;
    XAVFrame& operator=(const XAVFrame &);
    XAVFrame& operator=(XAVFrame && ) noexcept;
    ~XAVFrame();
};

using XAVFrame_sptr = std::shared_ptr<XAVFrame>;
XAVFrame_sptr new_XAVFrame() noexcept(false);

#endif
