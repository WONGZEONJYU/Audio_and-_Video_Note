//
// Created by Administrator on 2024/7/29.
//

#include "XAVFrame.hpp"

XAVFrame::XAVFrame() : AVFrame() {
    pts                   =
    pkt_dts               = AV_NOPTS_VALUE;
    best_effort_timestamp = AV_NOPTS_VALUE;
    duration            = 0;
    time_base           = { 0, 1 };
    sample_aspect_ratio = { 0, 1 };
    format              = -1; /* unknown */
    extended_data       = data;
    color_primaries     = AVCOL_PRI_UNSPECIFIED;
    color_trc           = AVCOL_TRC_UNSPECIFIED;
    colorspace          = AVCOL_SPC_UNSPECIFIED;
    color_range         = AVCOL_RANGE_UNSPECIFIED;
    chroma_location     = AVCHROMA_LOC_UNSPECIFIED;
    flags               = 0;
}

XAVFrame::XAVFrame(const XAVFrame &obj) :XAVFrame() {
    av_frame_ref(this,std::addressof(obj));
}

XAVFrame::XAVFrame(XAVFrame &&obj) noexcept : XAVFrame(){
    av_frame_move_ref(this,std::addressof(obj));
}

XAVFrame &XAVFrame::operator=(const XAVFrame &obj) {
    if (this != std::addressof(obj)){
        av_frame_ref(this,std::addressof(obj));
    }
    return *this;
}

XAVFrame &XAVFrame::operator=(XAVFrame &&obj) noexcept {
    if (this != std::addressof(obj)){
        av_frame_move_ref(this,std::addressof(obj));
    }
    return *this;
}

XAVFrame::~XAVFrame() {
    av_frame_unref(this);
}

XAVFrame_sptr new_XAVFrame() noexcept(false)
{
    try {
        return std::make_shared<XAVFrame>();
    } catch (...) {
        throw std::runtime_error(__func__ + std::string(" error!\n"));
    }
}
