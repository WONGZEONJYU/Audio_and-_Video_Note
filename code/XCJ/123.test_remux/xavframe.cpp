//
// Created by Administrator on 2024/7/29.
//

#include "xavframe.hpp"
#include "xhelper.hpp"
extern "C"{
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
}

XAVFrame::XAVFrame() : AVFrame() {
    av_frame_unref(this);
}

XAVFrame::XAVFrame(const AVFrame &frame) : AVFrame() {
    av_frame_ref(this,std::addressof(frame));
}

XAVFrame::XAVFrame(const AVFrame *frame) : AVFrame() {
    av_frame_ref(this,frame);
}

XAVFrame::XAVFrame(const XAVFrame &obj) :XAVFrame() {
    av_frame_ref(this,std::addressof(obj));
}

XAVFrame::XAVFrame(XAVFrame &&obj) noexcept(true) :XAVFrame() {
    av_frame_move_ref(this,std::addressof(obj));
}

XAVFrame &XAVFrame::operator=(const XAVFrame &obj) {
    auto obj_{std::addressof(obj)};
    if (this != obj_) {
        av_frame_unref(this);//先释放自身的再调用av_frame_ref
        av_frame_ref(this,obj_);
    }
    return *this;
}

XAVFrame &XAVFrame::operator=(XAVFrame &&obj) noexcept(true) {
    auto obj_{std::addressof(obj)};
    if (this != obj_) {
        av_frame_unref(this);//先释放自身再调用av_frame_move_ref
        av_frame_move_ref(this,obj_);
    }
    return *this;
}

XAVFrame::~XAVFrame() {
    av_frame_unref(this);
}

bool XAVFrame::Get_Buffer(const int &align) {
    FF_ERR_OUT(av_frame_get_buffer(this,align),return {});
    return true;
}

bool XAVFrame::Make_Writable() {
    FF_ERR_OUT(av_frame_make_writable(this),return {});
    return true;
}

void XAVFrame::Reset(AVFrame *frame) {
    av_frame_unref(this);
    if (frame){
        av_frame_move_ref(this,frame);
    }
}

int XAVFrame::Image_Fill_Arrays(const uint8_t *src,const int &fmt,
                                const int &w, const int &h,
                                const int &align) {
    int ret;
    FF_ERR_OUT(ret = av_image_fill_arrays(data,linesize,src,static_cast<AVPixelFormat>(fmt),w,h,align),return ret);
    return ret;
}

int XAVFrame::Samples_Fill_Arrays(const uint8_t *src,
                                  const int &nb_channels,
                                  const int &nb_samples,
                                  const int &sample_fmt,
                                    const int &align) {
    int ret;
    FF_ERR_OUT(ret = av_samples_fill_arrays(data,linesize,
                                            src,
                                            nb_channels,nb_samples,
                                            static_cast<AVSampleFormat>(sample_fmt),align),return ret);
    return ret;
}

XAVFrame_sptr new_XAVFrame() noexcept(false) {
    XAVFrame_sptr obj;
    CHECK_EXC(obj = std::make_shared<XAVFrame>());
    return obj;
}

