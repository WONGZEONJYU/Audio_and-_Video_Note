//
// Created by Administrator on 2024/7/31.
//
extern "C"{
#include <libavcodec/codec.h>
}
#include <thread>
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
        cerr << "XAVCodecParameters_sptr is empty\n";
        return;
    }

    Close();

    auto codec{avcodec_find_decoder(static_cast<AVCodecID>(parm->codec_id()))};
    if (!codec){
        cerr << "can't find the codec id: " << parm->codec_name() << "\n";
        return;
    }

    unique_lock lock(m_re_mux);
    try {
        CHECK_NULLPTR(m_codec_ctx = avcodec_alloc_context3(codec));
        m_codec_ctx->thread_count = static_cast<int>(thread::hardware_concurrency());
        parm->to_context(m_codec_ctx);
        FF_CHECK_ERR(avcodec_open2(m_codec_ctx, nullptr, nullptr));

    }catch (...) {
        lock.unlock();
        DeConstruct();
        rethrow_exception(current_exception());
    }
}

void XDecode::DeConstruct() noexcept(true) {
    avcodec_free_context(&m_codec_ctx);
}

void XDecode::Close() noexcept(false) {
    unique_lock lock(m_re_mux);
    DeConstruct();
}

void XDecode::Clear() noexcept(false) {
    unique_lock lock(m_re_mux);
    if (!m_codec_ctx){
        return;
    }
    avcodec_flush_buffers(m_codec_ctx);
}

[[maybe_unused]] [[nodiscard]] bool XDecode::Send(const XAVPacket_sptr &pkt) const noexcept(false) {

    bool b{};

    if (!m_codec_ctx){
        cerr << __func__  << ": m_codec_ctx is empty\n";
        return b;
    }

    auto ret{-1};
    FF_ERR_OUT(ret = avcodec_send_packet(m_codec_ctx,pkt.get()));
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

[[maybe_unused]] [[nodiscard]] XAVFrame_sptr XDecode::Receive() const noexcept(false) {

    XAVFrame_sptr frame;
    if (!m_codec_ctx){
        cerr << __func__  << "m_codec_ctx is empty\n";
        return frame;
    }

    CHECK_EXC(frame = new_XAVFrame());
    auto ret{-1};
    FF_ERR_OUT(ret = avcodec_receive_frame(m_codec_ctx, frame.get()));
    if (AVERROR(EAGAIN) == ret || AVERROR_EOF == ret || AVERROR(EINVAL) == ret){
        frame.reset();
        return frame;
    }else if (ret < 0){
        FF_CHECK_ERR(ret);
    }else{}
    return frame;
}
