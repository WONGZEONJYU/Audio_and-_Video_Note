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
     * 拷贝AVFrame对象,引用计数+1
     * @param frame
     */
    explicit XAVFrame(const AVFrame &frame);
    /**
     * 同上
     * @param frame
     */
    explicit XAVFrame(const AVFrame *frame);

    /**
     * 拷贝并非真拷贝,而是引用计数+1
     * 等价于av_frame_ref
     */
    XAVFrame(const XAVFrame &);

    /**
     * 移动构造
     * av_frame_move_ref
     */
    XAVFrame(XAVFrame &&) noexcept(true);

    /**
     * 引用计数+1
     * @return XAVFrame&
     */
    XAVFrame& operator=(const XAVFrame &);

    /**
     * 移动赋值
     * av_frame_move_ref
     * @return
     */
    XAVFrame& operator=(XAVFrame && ) noexcept(true);

    /**
     * 引用计数-1
     */
    ~XAVFrame();

    /**
     * 调用前,请先填写相关成员参数
     * 视频:填写width , height , format
     * 音频: format , ch_layout.nb_channels , nb_samples , sample_rate
     * 音频/视频 公共参数 linesize[]数组
     * @param align
     * @return true or false
     */
    bool Get_Buffer(const int &align = 0);

    /**
     * 确保帧数据可写,尽可能避免数据复制
     * 如果帧可写,则不执行任何操作
     * 如果不可写,则分配新缓冲区并复制数据
     * 非引用计数的帧表现为不可写,即始终进行复制
     * @return true or false
     */
    bool Make_Writable();

    /**
     * 释放本对象数据,如果frame不为空,则拷贝AVFrame,引用计数+1
     * @param frame
     */
    void Reset(AVFrame *frame = nullptr);

};

using XAVFrame_sptr = typename std::shared_ptr<XAVFrame>;
XAVFrame_sptr new_XAVFrame() noexcept(false);

#endif
