//
// Created by Administrator on 2024/9/7.
//
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

#include "xencode.hpp"
#include <thread>

AVCodecContext *XEncode::Create(const int &codec_id) {

    //1.查找编码器
    auto codec {avcodec_find_encoder(static_cast<AVCodecID>(codec_id))};
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

void XEncode::SetOpt(const std::string &key,const std::string &val){
    std::unique_lock locker(m_mux);
    if (!m_codec_ctx){
        PRINT_ERR_TIPS(GET_STR(AVCodecContext Not Created!));
        return;
    }
    av_opt_set(m_codec_ctx->priv_data,key.c_str(),val.c_str(),0);
}

void XEncode::SetOpt(const std::string &key,const int64_t &val){
    std::unique_lock locker(m_mux);
    if (!m_codec_ctx){
        PRINT_ERR_TIPS(GET_STR(AVCodecContext Not Created!));
        return;
    }
    av_opt_set_int(m_codec_ctx->priv_data,key.c_str(),val,0);
}

