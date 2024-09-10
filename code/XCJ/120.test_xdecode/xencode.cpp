//
// Created by Administrator on 2024/9/7.
//
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

#include "xencode.hpp"
#include <thread>
#include "XAVFrame.hpp"
#include "XAVPacket.hpp"

#define CHECK_CODEC_CTX \
std::unique_lock locker(m_mux);\
if (!m_codec_ctx){\
PRINT_ERR_TIPS(GET_STR(AVCodecContext Not Created!));\
return {};}

AVCodecContext *XEncode::Create(const int &codec_id) {

    //1.查找编码器
    auto codec{avcodec_find_encoder(static_cast<AVCodecID>(codec_id))};
    if (!codec){
        PRINT_ERR_TIPS(GET_STR(avcodec_find_encoder failed!));
        return {};
    }

    //2.分配解码器上下文
    auto codec_ctx{avcodec_alloc_context3(codec)};
    if (!codec_ctx){
        return {};
    }

    //3.设置默认参数
    codec_ctx->time_base = {1,25};
    codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    codec_ctx->thread_count = static_cast<int>(std::thread::hardware_concurrency());

    return codec_ctx;
}

void XEncode::set_codec_ctx(AVCodecContext *ctx) {
    std::unique_lock locker(m_mux);
    if (m_codec_ctx){
        avcodec_free_context(&m_codec_ctx);
    }
    m_codec_ctx = ctx;
}

#define CHECK_ENCODE_OPEN do{ \
if (avcodec_is_open(m_codec_ctx)){\
PRINT_ERR_TIPS(GET_STR((encode is open,Invalid parameter setting\n)));} \
}while(false)

bool XEncode::SetOpt(const std::string &key,const std::string &val){
    CHECK_CODEC_CTX
    CHECK_ENCODE_OPEN;
    FF_ERR_OUT(av_opt_set(m_codec_ctx->priv_data,key.c_str(),val.c_str(),0),return {});
    return true;
}

bool XEncode::SetOpt(const std::string &key,const int64_t &val){
    CHECK_CODEC_CTX
    CHECK_ENCODE_OPEN;
    FF_ERR_OUT(av_opt_set_int(m_codec_ctx->priv_data,key.c_str(),val,0),return {});
    return true;
}

bool XEncode::Set_Qp(const QP &qp){
    return SetOpt(QP::m_name,qp);
}

bool XEncode::Set_CRF(const CRF &crf){
    return SetOpt(CRF::m_name,crf.value());
}

bool XEncode::Open() {
    CHECK_CODEC_CTX
    FF_ERR_OUT(avcodec_open2(m_codec_ctx,{},{}),return {});
    return true;
}

XAVPacket_sptr XEncode::Encode(const XAVFrame_sptr &frame) {

    CHECK_CODEC_CTX
    auto ret{avcodec_send_frame(m_codec_ctx,frame.get())};
    if (0 != ret || AVERROR(EAGAIN) != ret ){
        FF_ERR_OUT(ret,return {});
    }

    XAVPacket_sptr pkt;
    TRY_CATCH(CHECK_EXC(pkt = new_XAVPacket()),return {});
    ret = avcodec_receive_packet(m_codec_ctx,pkt.get());
    if (ret < 0){
        FF_ERR_OUT(ret,pkt.reset());
    }
    return pkt;
}

std::vector<XAVPacket_sptr> XEncode::Flush() {
    CHECK_CODEC_CTX

    auto ret{avcodec_send_frame(m_codec_ctx,{})};
    if (ret < 0){
        FF_ERR_OUT(ret,return {});
    }

    std::vector<XAVPacket_sptr> packets;

    while (ret >= 0){
        XAVPacket_sptr pkt;
        TRY_CATCH(CHECK_EXC(pkt = new_XAVPacket()),return {});
        ret = avcodec_receive_packet(m_codec_ctx,pkt.get());
        if (ret < 0) {
            FF_ERR_OUT(ret);
            break;
        }
        packets.push_back(std::move(pkt));
    }

    return packets;
}

XAVFrame_sptr XEncode::Alloc_AVFrame(){

    CHECK_CODEC_CTX
    XAVFrame_sptr frame;
    TRY_CATCH(CHECK_EXC(frame = new_XAVFrame()),return {});
    frame->width = m_codec_ctx->width;
    frame->height = m_codec_ctx->height;
    frame->format = m_codec_ctx->pix_fmt;
    FF_ERR_OUT(frame->Get_Buffer(),frame.reset(); return {});
    return frame;
}

XEncode::~XEncode() {
    std::unique_lock locker(m_mux);
    avcodec_free_context(&m_codec_ctx);
}


