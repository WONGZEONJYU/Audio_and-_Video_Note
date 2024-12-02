extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

#include "xcodec.hpp"
#include "xavframe.hpp"

using namespace std;

AVCodecContext *XCodec::Create(const int &codec_id,const bool &is_encode) {

    const auto codec {is_encode ? avcodec_find_encoder(static_cast<AVCodecID>(codec_id)) :
        avcodec_find_decoder(static_cast<AVCodecID>(codec_id))};
    // codec = is_encode
    // if (is_encode){
    //     //1.查找编码器
    //     codec = avcodec_find_encoder(static_cast<AVCodecID>(codec_id));
    // } else{
    //     //1.查找解码器
    //     codec = avcodec_find_decoder(static_cast<AVCodecID>(codec_id));
    // }

    if (!codec){
        PRINT_ERR_TIPS(GET_STR(codec not found!));
        return {};
    }

    //2.分配编解码器上下文
    AVCodecContext *codec_ctx{};
    IS_NULLPTR(codec_ctx = avcodec_alloc_context3(codec),return {});

    //3.设置参数,编码
    codec_ctx->time_base= {1,25};
    codec_ctx->gop_size = 25;
    codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    const auto thread_count{static_cast<int>(thread::hardware_concurrency())};
    codec_ctx->thread_count = thread_count > 16 ? 16 : thread_count;
    return codec_ctx;
}

void XCodec::set_codec_ctx(AVCodecContext *ctx) {
    unique_lock locker(m_mux_);
    if (m_codec_ctx_){
        avcodec_free_context(&m_codec_ctx_);
    }
    m_codec_ctx_ = ctx;
}

bool XCodec::SetOpt(const string &key,const string &val) const{
    CHECK_CODEC_CTX_RET();
    CHECK_ENCODE_OPEN();
    FF_ERR_OUT(av_opt_set(m_codec_ctx_->priv_data,key.c_str(),val.c_str(),0),return {});
    return true;
}

bool XCodec::SetOpt(const string &key,const int64_t &val) const {
    CHECK_CODEC_CTX_RET();
    CHECK_ENCODE_OPEN();
    FF_ERR_OUT(av_opt_set_int(m_codec_ctx_->priv_data,key.c_str(),val,0),return {});
    return true;
}

bool XCodec::Open() {
    CHECK_CODEC_CTX_RET();
    FF_ERR_OUT(avcodec_open2(m_codec_ctx_,{},{}),
               avcodec_free_context(&m_codec_ctx_);return {});
    return true;
}

void XCodec::Clear(){
    CHECK_CODEC_CTX();
    avcodec_flush_buffers(m_codec_ctx_);
}

XAVFrame_sp XCodec::CreateFrame(const int &align) const{
    CHECK_CODEC_CTX_RET();
    XAVFrame_sp frame;
    IS_SMART_NULLPTR(frame = new_XAVFrame(),return {});
    frame->width = m_codec_ctx_->width;
    frame->height = m_codec_ctx_->height;
    frame->format = m_codec_ctx_->pix_fmt;
    if (!frame->Get_Buffer(align)) {
        frame.reset();
    }
    return frame;
}

void XCodec::destroy(){
    unique_lock locker(m_mux_);
    avcodec_free_context(&m_codec_ctx_);
}

XCodec::~XCodec() {
    destroy();
}
