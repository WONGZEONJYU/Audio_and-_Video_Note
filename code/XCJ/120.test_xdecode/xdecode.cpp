//
// Created by Administrator on 2024/9/10.
//

#include "xdecode.hpp"

extern "C"{
#include <libavcodec/avcodec.h>
}

#include "xavpacket.hpp"
#include "xavframe.hpp"

#define CHECK_CODEC_CTX \
std::unique_lock locker(m_mux);\
if (!m_codec_ctx){\
PRINT_ERR_TIPS(GET_STR(AVCodecContext Not Created!));\
return {};}

bool XDecode::Send(const XAVPacket_sptr &packet) {
    if (!packet){
        PRINT_ERR_TIPS(GET_STR(packet is empty!));
        return {};
    }
    CHECK_CODEC_CTX
    FF_ERR_OUT(avcodec_send_packet(m_codec_ctx,packet.get()),return {});
    return true;
}

bool XDecode::Receive(XAVFrame_sptr &frame) {
    if (!frame){
        PRINT_ERR_TIPS(GET_STR(frame is empty!));
        return {};
    }
    CHECK_CODEC_CTX
    FF_ERR_OUT(avcodec_receive_frame(m_codec_ctx,frame.get()),return {});
    return true;
}

XAVFrames XDecode::Flush() {
    CHECK_CODEC_CTX
    FF_ERR_OUT(avcodec_send_packet(m_codec_ctx,{}),return {});
    XAVFrames frames;
    while (true) {
        XAVFrame_sptr frame;
        TRY_CATCH(CHECK_EXC(frame = new_XAVFrame()),return frames);
        FF_ERR_OUT(avcodec_receive_frame(m_codec_ctx,frame.get()),return frames);
        frames.push_back(std::move(frame));
    }
}
