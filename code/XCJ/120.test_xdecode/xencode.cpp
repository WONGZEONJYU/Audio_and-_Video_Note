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
    const auto ret{avcodec_send_frame(m_codec_ctx,frame.get())};
    if (0 != ret || AVERROR(EAGAIN) != ret){
        FF_ERR_OUT(ret,return {});
    }

    XAVPacket_sptr packet;
    TRY_CATCH(CHECK_EXC(packet = new_XAVPacket()),return {});
    FF_ERR_OUT(avcodec_receive_packet(m_codec_ctx,packet.get()),packet.reset());
    return packet;
}

XAVPackets XEncode::Flush() {
    CHECK_CODEC_CTX
    FF_ERR_OUT(avcodec_send_frame(m_codec_ctx,{}),return {});
    std::vector<XAVPacket_sptr> packets;
    while (true){
        XAVPacket_sptr packet;
        TRY_CATCH(CHECK_EXC(packet = new_XAVPacket()),return packets);
        FF_ERR_OUT(avcodec_receive_packet(m_codec_ctx,packet.get()),return packets);
        packets.push_back(std::move(packet));
    }
}

