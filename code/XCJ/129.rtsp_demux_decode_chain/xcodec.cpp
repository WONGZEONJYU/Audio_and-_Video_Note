extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

#include <thread>
#include "xcodec.hpp"
#include "xavframe.hpp"

AVCodecContext *XCodec::Create(const int &codec_id,const bool &is_encode) {

    const AVCodec *codec{};

    if (is_encode){
        //1.查找编码器
        codec = avcodec_find_encoder(static_cast<AVCodecID>(codec_id));
    } else{
        //1.查找解码器
        codec = avcodec_find_decoder(static_cast<AVCodecID>(codec_id));
    }

    if (!codec){
        PRINT_ERR_TIPS(GET_STR(codec not found!));
        return {};
    }

    //2.分配编解码器上下文
    AVCodecContext *codec_ctx{};
    IS_NULLPTR(codec_ctx = avcodec_alloc_context3(codec),return {});

    //3.设置参数,解码
    codec_ctx->time_base = {1,25};
    codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    codec_ctx->thread_count = static_cast<int>(std::thread::hardware_concurrency());

    return codec_ctx;
}

void XCodec::set_codec_ctx(AVCodecContext *ctx) {
    std::unique_lock locker(m_mux_);
    if (m_codec_ctx_){
        avcodec_free_context(&m_codec_ctx_);
    }
    m_codec_ctx_ = ctx;
}

#define CHECK_ENCODE_OPEN() do{ \
if (avcodec_is_open(m_codec_ctx_)){\
PRINT_ERR_TIPS(GET_STR((encode is open,Invalid parameter setting\n)));} \
}while(false)

bool XCodec::SetOpt(const std::string &key,const std::string &val){
    CHECK_CODEC_CTX();
    CHECK_ENCODE_OPEN();
    FF_ERR_OUT(av_opt_set(m_codec_ctx_->priv_data,key.c_str(),val.c_str(),0),return {});
    return true;
}

bool XCodec::SetOpt(const std::string &key,const int64_t &val){
    CHECK_CODEC_CTX();
    CHECK_ENCODE_OPEN();
    FF_ERR_OUT(av_opt_set_int(m_codec_ctx_->priv_data,key.c_str(),val,0),return {});
    return true;
}

bool XCodec::Set_Qp(const QP &qp){
    return SetOpt(QP::m_name,qp);
}

bool XCodec::Set_CRF(const CRF &crf){
    return SetOpt(CRF::m_name,crf.value());
}

bool XCodec::Open() {
    CHECK_CODEC_CTX();
    FF_ERR_OUT(avcodec_open2(m_codec_ctx_,{},{}),
               avcodec_free_context(&m_codec_ctx_);return {});
    return true;
}

XAVFrame_sp XCodec::CreateFrame() const{

    CHECK_CODEC_CTX();
    XAVFrame_sp frame;
    IS_SMART_NULLPTR(frame = new_XAVFrame(),return {});
    frame->width = m_codec_ctx_->width;
    frame->height = m_codec_ctx_->height;
    frame->format = m_codec_ctx_->pix_fmt;
    FF_ERR_OUT(frame->Get_Buffer(),frame.reset(); return {});
    return frame;
}

void XCodec::destroy(){
    std::unique_lock locker(m_mux_);
    avcodec_free_context(&m_codec_ctx_);
}

XCodec::~XCodec() {
    destroy();
}
