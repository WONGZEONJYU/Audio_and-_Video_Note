//
// Created by Administrator on 2024/7/29.
//

#ifndef XPLAY2_XAVFRAME_HPP
#define XPLAY2_XAVFRAME_HPP

extern "C"{
#include <libavutil/frame.h>
}
#include <memory>

class XAVFrame final : public AVFrame {

public:
    XAVFrame();
    /**
     * 拷贝并非真拷贝,而是引用计数+1
     */
    XAVFrame(const XAVFrame &);
    XAVFrame(XAVFrame &&) noexcept(true);
    /**
     * 引用计数+1
     * @return XAVFrame&
     */
    XAVFrame& operator=(const XAVFrame &);
    XAVFrame& operator=(XAVFrame && ) noexcept(true);
    ~XAVFrame();
};

using XAVFrame_sptr = std::shared_ptr<XAVFrame>;
XAVFrame_sptr new_XAVFrame() noexcept(false);

#endif
