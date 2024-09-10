//
// Created by Administrator on 2024/7/29.
//

#include "xavframe.hpp"
#include "xhelper.hpp"

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

XAVFrame_sptr new_XAVFrame() noexcept(false) {
    XAVFrame_sptr obj;
    CHECK_EXC(obj = std::make_shared<XAVFrame>());
    return obj;
}
