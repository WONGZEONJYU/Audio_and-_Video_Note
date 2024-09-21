//
// Created by Administrator on 2024/9/10.
//

#include "xdecode.hpp"

extern "C"{
#include <libavcodec/avcodec.h>
}

#include "xavpacket.hpp"
#include "xavframe.hpp"

#define CHECK_CODEC_CTX() \
std::unique_lock locker(m_mux);\
if (!m_codec_ctx){\
PRINT_ERR_TIPS(GET_STR(AVCodecContext Not Created!));\
return {};}

bool XDecode::Send(const XAVPacket* packet) {
    if (!packet){
        PRINT_ERR_TIPS(GET_STR(packet is empty!));
        return {};
    }
    CHECK_CODEC_CTX()
    FF_ERR_OUT(avcodec_send_packet(m_codec_ctx,packet),return {});
    return true;
}

bool XDecode::Receive(XAVFrame *frame) {
    if (!frame){
        PRINT_ERR_TIPS(GET_STR(frame is empty!));
        return {};
    }
    CHECK_CODEC_CTX()
    FF_ERR_OUT(avcodec_receive_frame(m_codec_ctx,frame),return {});
    if(m_codec_ctx->hw_device_ctx) {
        XAVFrame_sptr hw_frame;
        TRY_CATCH(CHECK_EXC(hw_frame = new_XAVFrame()),return {});
        FF_ERR_OUT(av_hwframe_transfer_data(hw_frame.get(),frame,0), return {});
        frame->Reset(hw_frame.get());
    }
    return true;
}

XAVFrames XDecode::Flush() {
    CHECK_CODEC_CTX()
    FF_ERR_OUT(avcodec_send_packet(m_codec_ctx,{}),return {});
    XAVFrames frames;
    while (true) {
        XAVFrame_sptr frame;
        TRY_CATCH(CHECK_EXC(frame = new_XAVFrame()),return frames);
        FF_ERR_OUT(avcodec_receive_frame(m_codec_ctx,frame.get()),return frames);
        if (m_codec_ctx->hw_device_ctx){
            XAVFrame_sptr hw_frame;
            TRY_CATCH(CHECK_EXC(hw_frame = new_XAVFrame()),return frames);
            FF_ERR_OUT(av_hwframe_transfer_data(hw_frame.get(),frame.get(),0), return frames);
            frame = std::move(hw_frame);
        }
        frames.push_back(std::move(frame));
    }
}

bool XDecode::InitHw(const int &type) {

    CHECK_CODEC_CTX()

    if(avcodec_is_open(m_codec_ctx)){
        PRINT_ERR_TIPS(GET_STR(Please set before opening the decoder!));
        return {};
    }

    const auto hw_type{static_cast<AVHWDeviceType>(type)};
    AVBufferRef *hw_device_ctx{}; //硬件加速
    FF_ERR_OUT(av_hwdevice_ctx_create(&hw_device_ctx,hw_type,{},{},0),return {});
    //m_codec_ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
    m_codec_ctx->hw_device_ctx = hw_device_ctx;
    std::cerr << "Hardware Acceleration: " << av_hwdevice_get_type_name(hw_type) << "\n";
    return true;
}
