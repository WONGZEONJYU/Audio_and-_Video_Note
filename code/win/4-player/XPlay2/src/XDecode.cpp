//
// Created by Administrator on 2024/7/31.
//
extern "C"{
#include <libavcodec/avcodec.h>
}

#include <thread>
#include <iostream>
#include "XDecode.hpp"
#include "XAVCodecParameters.hpp"
#include "XAVPacket.hpp"
#include "XAVFrame.hpp"

using namespace std;

XDecode::~XDecode() {
    DeConstruct();
}

void XDecode::Open(const XAVCodecParameters_sptr &parm) noexcept(false) {

    if (!parm){
        PRINT_ERR_TIPS(GET_STR(XAVCodecParameters_sptr is empty));
        return;
    }

    Close();

    auto codec{avcodec_find_decoder(static_cast<AVCodecID>(parm->Codec_id()))};
    if (!codec){
        cerr << "can't find the codec id: " << parm->Codec_name() << "\n";
        return;
    }

    unique_lock lock(m_mux);
    try {
        CHECK_NULLPTR(m_codec_ctx = avcodec_alloc_context3(nullptr));
        m_codec_ctx->thread_count = static_cast<int>(thread::hardware_concurrency());
        parm->to_context(m_codec_ctx);
        FF_CHECK_ERR(avcodec_open2(m_codec_ctx, codec, nullptr));
    }catch (...) {
        lock.unlock();
        DeConstruct();
        throw;
    }
}

void XDecode::DeConstruct() noexcept(true) {
    m_pts = 0;
    avcodec_free_context(&m_codec_ctx);
}

void XDecode::Close() noexcept(false) {
    unique_lock lock(m_mux);
    DeConstruct();
}

void XDecode::Clear() noexcept(false) {
    unique_lock lock(m_mux);
    if (!m_codec_ctx){
        PRINT_ERR_TIPS(GET_STR(Please initialize first));
        return;
    }
    avcodec_flush_buffers(m_codec_ctx);
}

bool XDecode::Send(const XAVPacket_sptr &pkt)  noexcept(false) {

    int ret;
    {
        unique_lock lock(m_mux);
        if (!m_codec_ctx){
            PRINT_ERR_TIPS(GET_STR(Please initialize first));
            return {};
        }
        FF_ERR_OUT(ret = avcodec_send_packet(m_codec_ctx,pkt.get()));
        //ret = avcodec_send_packet(m_codec_ctx,pkt.get());
    }

    bool b{};
    if (AVERROR_EOF == ret || AVERROR(EAGAIN) == ret ||
        AVERROR(EINVAL) == ret || AVERROR(ENOMEM) == ret) {
        return b;
    }else if (ret < 0){
        FF_CHECK_ERR(ret);
    } else{
        b = true;
    }
    return b;
}

XAVFrame_sptr XDecode::Receive() noexcept(false) {

    unique_lock lock(m_mux);
    if (!m_codec_ctx){
        PRINT_ERR_TIPS(GET_STR(Please initialize first));
        return {};
    }

    XAVFrame_sptr frame;
    CHECK_EXC(frame = new_XAVFrame(),lock.unlock());

    const auto ret {avcodec_receive_frame(m_codec_ctx, frame.get())};
    lock.unlock();
    if (AVERROR(EAGAIN) == ret || AVERROR_EOF == ret || AVERROR(EINVAL) == ret){
        frame.reset();
        FF_ERR_OUT(ret);
//        if (AVERROR(EINVAL) == ret){
//            FF_ERR_OUT(ret);
//        }
    }else if (ret < 0){ //其他错误抛异常
        frame.reset();
        FF_CHECK_ERR(ret);
    }else{
        m_pts = frame->pts;
    }
    return frame;
}
