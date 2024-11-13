#ifndef XAVFRAME_HPP_
#define XAVFRAME_HPP_

extern "C"{
#include <libavutil/frame.h>
}

#include "xhelper.hpp"

#ifdef _MSVC_LANG
#pragma execution_character_set("utf-8")
#endif

class XLIB_API XAVFrame final : public AVFrame {

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
     * 图像帧填充
     * @param src
     * @param fmt
     * @param w
     * @param h
     * @param align
     * @return
     */
    int Image_Fill_Arrays(const uint8_t *src,
                          const int &fmt,
                          const int &w,
                          const int &h,
                          const int &align = 0);

    int Samples_Fill_Arrays(const uint8_t *src,const int &nb_channels,const int &nb_samples,
                            const int &sample_fmt,const int &align = 0);

    /**
     * 释放本对象数据,如果frame不为空,则拷贝AVFrame,引用计数+1
     * @param frame
     */
    void Reset(const AVFrame *frame = nullptr);

    /**
     * 对AVFrame进行引用计数+1,如果frame == null,则不做任何操作
     * @param frame
     */
    void Ref_fromAVFrame(const AVFrame *frame);
    void Ref_fromAVFrame(const AVFrame &frame);

    /**
     * 转移AVFrame的引用计数,被转移后的AVFrame谨慎使用
     * @param frame
     */
    void Move_fromAVFrame(AVFrame *frame);
    void Move_fromAVFrame(AVFrame &&frame);

    explicit operator bool() const;
    bool operator !() const;
    [[nodiscard]] bool empty() const;

};

XLIB_API XAVFrame_sp new_XAVFrame() noexcept(true);
XLIB_API XAVFrame_sp new_XAVFrame(const AVFrame &frame) noexcept(true);
XLIB_API XAVFrame_sp new_XAVFrame(const AVFrame *frame) noexcept(true);

#endif
