//
// Created by Administrator on 2024/9/7.
//
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

#include "xencode.hpp"
#include <thread>
#include "xavframe.hpp"
#include "xavpacket.hpp"

#define CHECK_CODEC_CTX \
std::unique_lock locker(m_mux);\
if (!m_codec_ctx){\
PRINT_ERR_TIPS(GET_STR(AVCodecContext Not Created!));\
return {};}

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

XAVPackets XEncode::Flush() {
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

