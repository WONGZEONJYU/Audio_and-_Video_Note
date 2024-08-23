//
// Created by Administrator on 2024/7/30.
//

extern "C"{
#include <libavcodec/avcodec.h>
}

#include "XAVCodecParameters.hpp"
#include <iostream>
#include <algorithm>

void XAVCodecParameters::Reset(AVCodecParameters *obj) noexcept(true) {
    av_freep(&obj->extradata);
    av_channel_layout_uninit(&obj->ch_layout);
    av_packet_side_data_free(&obj->coded_side_data, &obj->nb_coded_side_data);

    std::fill_n(reinterpret_cast<uint8_t *>(obj), sizeof(AVCodecParameters),0);

    obj->codec_type          = AVMEDIA_TYPE_UNKNOWN;
    obj->codec_id            = AV_CODEC_ID_NONE;
    obj->format              = -1;
    obj->ch_layout.order     = AV_CHANNEL_ORDER_UNSPEC;
    obj->field_order         = AV_FIELD_UNKNOWN;
    obj->color_range         = AVCOL_RANGE_UNSPECIFIED;
    obj->color_primaries     = AVCOL_PRI_UNSPECIFIED;
    obj->color_trc           = AVCOL_TRC_UNSPECIFIED;
    obj->color_space         = AVCOL_SPC_UNSPECIFIED;
    obj->chroma_location     = AVCHROMA_LOC_UNSPECIFIED;
    obj->sample_aspect_ratio = { 0, 1 };
    obj->framerate           = { 0, 1 };
    obj->profile             = AV_PROFILE_UNKNOWN;
    obj->level               = AV_LEVEL_UNKNOWN;
}

void XAVCodecParameters::Move(AVCodecParameters *obj) noexcept(true) {
    auto src_{obj};
    auto dst_{static_cast<decltype(src_)>(this)};
    Reset(dst_);//先释放自身的数据,再进行移动
    *dst_ = *src_;
    std::fill_n(reinterpret_cast<uint8_t*>(src_), sizeof(AVCodecParameters),0);
    //std::fill_n此处无法省掉,避免成员extradata,ch_layout,coded_side_data成员被直接释放掉
    Reset(src_);
}

XAVCodecParameters::XAVCodecParameters()
: AVCodecParameters(){
    Reset(this);
}

XAVCodecParameters::XAVCodecParameters(const AVCodecContext *src) noexcept(false)
:XAVCodecParameters(){
    from_context(src);
}

XAVCodecParameters::XAVCodecParameters(const AVCodecParameters *src) noexcept(false)
:XAVCodecParameters(){
    from_AVFormatContext(src);
}

XAVCodecParameters::XAVCodecParameters(const XAVCodecParameters &obj) noexcept(false)
:XAVCodecParameters() {
    from_AVFormatContext(std::addressof(obj));
}

XAVCodecParameters::XAVCodecParameters(XAVCodecParameters &&obj) noexcept(true)
: XAVCodecParameters(){
    Move(std::addressof(obj));
}

XAVCodecParameters &XAVCodecParameters::operator=(const XAVCodecParameters &obj) noexcept(false) {

    auto obj_{std::addressof(obj)};
    if (this != obj_){
        from_AVFormatContext(obj_);
    }
    return *this;
}

XAVCodecParameters& XAVCodecParameters::operator=(XAVCodecParameters &&obj) noexcept(true) {

    auto obj_{std::addressof(obj)};
    if (this != obj_){
        Move(obj_);
    }
    return *this;
}

void XAVCodecParameters::from_AVFormatContext(const AVCodecParameters *src) noexcept(false) {
    if (!src){
        PRINT_ERR_TIPS(GET_STR(src is nullptr));
        return;
    }
    FF_CHECK_ERR(avcodec_parameters_copy(this, src));
}

void XAVCodecParameters::from_context(const AVCodecContext *src)  noexcept(false) {
    if (!src){
        PRINT_ERR_TIPS(GET_STR(src is nullptr));
        return;
    }
    FF_CHECK_ERR(avcodec_parameters_from_context(this,src));
}

void XAVCodecParameters::to_context(AVCodecContext *dst) const noexcept(false) {
    if (!dst){
        PRINT_ERR_TIPS(GET_STR(dst is nullptr));
        return;
    }
    FF_CHECK_ERR(avcodec_parameters_to_context(dst,this));
}

std::string XAVCodecParameters::Codec_name() const noexcept(true) {
    return avcodec_get_name(AVCodecParameters::codec_id);
}

XAVCodecParameters::~XAVCodecParameters() {
    Reset(this);
}

XAVCodecParameters_sptr new_XAVCodecParameters() noexcept(false) {

    XAVCodecParameters_sptr obj;
    CHECK_EXC(obj = std::make_shared<XAVCodecParameters>());
    return obj;
}

XAVCodecParameters_sptr new_XAVCodecParameters(const AVCodecContext *src) noexcept(false) {
    XAVCodecParameters_sptr obj;
    CHECK_EXC(obj = std::make_shared<XAVCodecParameters>(src));
    return obj;
}

XAVCodecParameters_sptr new_XAVCodecParameters(const AVCodecParameters *src) noexcept(false){
    XAVCodecParameters_sptr obj;
    CHECK_EXC(obj = std::make_shared<XAVCodecParameters>(src));
    return obj;
}

int XAVCodecParameters::Sample_Format_Size() const noexcept(true){
    return av_get_bytes_per_sample(static_cast<AVSampleFormat>(format));
}
