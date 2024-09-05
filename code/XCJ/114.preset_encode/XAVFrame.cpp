//
// Created by Administrator on 2024/7/29.
//

#include "XAVFrame.hpp"
#include "XHelper.hpp"

XAVFrame::XAVFrame() : AVFrame() {
    av_frame_unref(this);
}

XAVFrame::XAVFrame(const XAVFrame &obj) :XAVFrame() {
    av_frame_ref(this,std::addressof(obj));
}

XAVFrame::XAVFrame(XAVFrame &&obj) noexcept(true) : XAVFrame(){
    av_frame_move_ref(this,std::addressof(obj));
}

XAVFrame &XAVFrame::operator=(const XAVFrame &obj) {
    auto obj_{std::addressof(obj)};
    if (this != obj_){
        av_frame_unref(this);//先释放自身的再调用av_frame_ref
        av_frame_ref(this,obj_);
    }
    return *this;
}

XAVFrame &XAVFrame::operator=(XAVFrame &&obj) noexcept(true) {
    auto obj_{std::addressof(obj)};
    if (this != obj_){
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

XAVFrame_sptr new_XAVFrame() noexcept(false) {
    XAVFrame_sptr obj;
    CHECK_EXC(obj = std::make_shared<XAVFrame>());
    return obj;
}
