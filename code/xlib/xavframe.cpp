#include "xavframe.hpp"

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
    if (const auto obj_{std::addressof(obj)}; this != obj_) {
        av_frame_unref(this);//先释放自身的再调用av_frame_ref
        av_frame_ref(this,obj_);
    }
    return *this;
}

XAVFrame &XAVFrame::operator=(XAVFrame &&obj) noexcept(true) {
    if (const auto obj_{std::addressof(obj)}; this != obj_) {
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

void XAVFrame::Reset(const AVFrame *frame) {
    av_frame_unref(this);
    if (frame){
        av_frame_ref(this,frame);
    }
}

void XAVFrame::Ref_fromAVFrame(const AVFrame *frame) {
    if (frame) {
        av_frame_unref(this);
        av_frame_ref(this,frame);
    }
}

void XAVFrame::Ref_fromAVFrame(const AVFrame &frame) {
    Ref_fromAVFrame(std::addressof(frame));
}

void XAVFrame::Move_fromAVFrame(AVFrame *frame) {
    if (frame) {
        av_frame_unref(this);
        av_frame_move_ref(this,frame);
    }
}

void XAVFrame::Move_fromAVFrame(AVFrame &&frame) {
    Move_fromAVFrame(std::addressof(frame));
}

int XAVFrame::Image_Fill_Arrays(const uint8_t *src,const int &fmt,
                                const int &w, const int &h,
                                const int &align) {
    int ret;
    FF_ERR_OUT(ret = av_image_fill_arrays(data,linesize,src,static_cast<AVPixelFormat>(fmt),w,h,align));
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
                                            static_cast<AVSampleFormat>(sample_fmt),align));
    return ret;
}

XAVFrame::operator bool() const {
    return data[0];
}

bool XAVFrame::operator!() const {
    return !data[0];
}

bool XAVFrame::empty() const {
    return !data[0];
}

XAVFrame_sp new_XAVFrame() noexcept(true) {
    XAVFrame_sp obj;
    TRY_CATCH(CHECK_EXC(obj = std::make_shared<XAVFrame>()),return {});
    return obj;
}

XAVFrame_sp new_XAVFrame(const AVFrame &frame) noexcept(true) {
    XAVFrame_sp obj;
    TRY_CATCH(CHECK_EXC(obj = std::make_shared<XAVFrame>(frame)),return {});
    return obj;
}

XAVFrame_sp new_XAVFrame(const AVFrame *frame) noexcept(true) {
    XAVFrame_sp obj;
    TRY_CATCH(CHECK_EXC(obj = std::make_shared<XAVFrame>(frame)),return {});
    return obj;
}
