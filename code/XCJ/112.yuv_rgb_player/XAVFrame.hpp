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

    /**
     * 调用前,请先填写相关成员参数
     * 视频:填写width , height , format
     * 音频: format , ch_layout.nb_channels , nb_samples , sample_rate
     * 音频/视频 公共参数 linesize[]数组
     * @param align
     * @return true or false
     */
    bool Get_Buffer(const int &align);
};

using XAVFrame_sptr = typename std::shared_ptr<XAVFrame>;
XAVFrame_sptr new_XAVFrame() noexcept(false);

#endif
