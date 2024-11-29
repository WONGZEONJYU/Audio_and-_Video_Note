extern "C"{
#include <libavcodec/avcodec.h>
}
#include "xcodec_parameters.hpp"

void XCodecParameters::Reset(AVCodecParameters *obj) noexcept(true) {
    av_freep(&obj->extradata);
    av_channel_layout_uninit(&obj->ch_layout);
    av_packet_side_data_free(&obj->coded_side_data,&obj->nb_coded_side_data);

    std::fill_n(reinterpret_cast<uint8_t *>(obj),sizeof(AVCodecParameters),0);

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

void XCodecParameters::Move(XCodecParameters *obj) noexcept(true) {
    auto src_{static_cast<AVCodecParameters*>(obj)};
    const auto dst_{static_cast<decltype(src_)>(this)};
    Reset(dst_);//先释放自身的数据,再进行移动
    *dst_ = *src_;
    m_time_base_ = obj->m_time_base_;
    m_x_time_base_ = obj->m_x_time_base_;
    m_total_ms_ = obj->m_total_ms_;
    obj->m_time_base_ = {1,1};
    obj->m_x_time_base_ = {1,1};
    obj->m_total_ms_ = 0;
    std::fill_n(reinterpret_cast<uint8_t*>(src_),sizeof(AVCodecParameters),0);
    //std::fill_n此处无法省掉,避免成员extradata,ch_layout,coded_side_data成员被直接释放掉
    //和对某些成员进行初始化
    Reset(src_);
}

XCodecParameters::XCodecParameters()
: AVCodecParameters(){
    Reset(this);
}

XCodecParameters::XCodecParameters(const AVCodecContext *src,const AVRational &tb,const int64_t &total_ms) noexcept(false)
:XCodecParameters(){
    from_context(src);
    m_time_base_ = tb;
    m_x_time_base_ = {tb.num,tb.den};
    m_total_ms_ = total_ms;
}

XCodecParameters::XCodecParameters(const AVCodecParameters *src,const AVRational &tb,const int64_t &total_ms) noexcept(false)
:XCodecParameters(){
    from_AVFormatContext(src);
    m_time_base_ = tb;
    m_x_time_base_ = {tb.num,tb.den};
    m_total_ms_ = total_ms;
}

XCodecParameters::XCodecParameters(const XCodecParameters &obj) noexcept(false)
:XCodecParameters() {
    from_AVFormatContext(std::addressof(obj));
    m_time_base_ = obj.m_time_base_;
    m_x_time_base_ = obj.m_x_time_base_;
    m_total_ms_ = obj.m_total_ms_;
}

XCodecParameters::XCodecParameters(XCodecParameters &&obj) noexcept(true)
: XCodecParameters(){
    Move(std::addressof(obj));
}

XCodecParameters &XCodecParameters::operator=(const XCodecParameters &obj) noexcept(false) {
    if (const auto obj_{std::addressof(obj)}; this != obj_){
        from_AVFormatContext(obj_);
        m_time_base_ = obj.m_time_base_;
        m_x_time_base_ = obj.m_x_time_base_;
        m_total_ms_ = obj.m_total_ms_;
    }
    return *this;
}

XCodecParameters& XCodecParameters::operator=(XCodecParameters &&obj) noexcept(true) {
    if (const auto obj_{std::addressof(obj)}; this != obj_){
        Move(obj_);
    }
    return *this;
}

bool XCodecParameters::from_AVFormatContext(const AVCodecParameters *src) noexcept(true) {
    if (!src){
        PRINT_ERR_TIPS(GET_STR(src is nullptr));
        return {};
    }
    FF_ERR_OUT(avcodec_parameters_copy(this, src),return {});
    return true;
}

bool XCodecParameters::to_AVCodecParameters(AVCodecParameters *dst) const noexcept(true){
    if (!dst){
        PRINT_ERR_TIPS(GET_STR(dst is nullptr));
        return {};
    }
    FF_ERR_OUT(avcodec_parameters_copy(dst,this),return {});
    return true;
}

bool XCodecParameters::from_context(const AVCodecContext *src)  noexcept(true) {
    if (!src){
        PRINT_ERR_TIPS(GET_STR(src is nullptr));
        return {};
    }
    FF_ERR_OUT(avcodec_parameters_from_context(this,src),return {});
    return true;
}

bool XCodecParameters::to_context(AVCodecContext *dst) const noexcept(true) {
    if (!dst){
        PRINT_ERR_TIPS(GET_STR(dst is nullptr));
        return {};
    }
    FF_ERR_OUT(avcodec_parameters_to_context(dst,this),return {});
    return true;
}

std::string XCodecParameters::Codec_name() const noexcept(true) {
    return avcodec_get_name(codec_id);
}

int XCodecParameters::Sample_Format_Size() const noexcept(true){
    return av_get_bytes_per_sample(static_cast<AVSampleFormat>(format));
}

int XCodecParameters::Audio_nbSamples() const noexcept(true) {
    return frame_size;
}

XCodecParameters::~XCodecParameters() {
    Reset(this);
}

XCodecParameters_sp new_XCodecParameters(){
    XCodecParameters_sp obj;
    TRY_CATCH(CHECK_EXC(obj = std::make_shared<XCodecParameters>()),return {});
    return obj;
}

XCodecParameters_sp new_XCodecParameters(const AVCodecParameters *src,
    const AVRational &tb,
    const int64_t &total_ms){
    XCodecParameters_sp obj;
    TRY_CATCH(CHECK_EXC(obj = std::make_shared<XCodecParameters>(src,tb,total_ms)),return {});
    return obj;
}

XCodecParameters_sp new_XCodecParameters(const AVCodecParameters *src,
    const XRational &tb,
    const int64_t &total_ms ) {
    return  new_XCodecParameters(src,AVRational{tb.num,tb.den},total_ms);
}

XCodecParameters_sp new_XCodecParameters(const AVCodecContext *src,
    const AVRational &tb,
    const int64_t &total_ms){
    XCodecParameters_sp obj;
    TRY_CATCH(CHECK_EXC(obj = std::make_shared<XCodecParameters>(src,tb,total_ms)),return {});
    return obj;
}

XCodecParameters_sp new_XCodecParameters(const AVCodecContext *src,
    const XRational &tb,
    const int64_t &total_ms ) {
    return new_XCodecParameters(src,AVRational{tb.num,tb.den},total_ms);
}
