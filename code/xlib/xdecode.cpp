extern "C"{
#include <libavcodec/avcodec.h>
}

#include "xdecode.hpp"
#include "xavpacket.hpp"
#include "xavframe.hpp"
#include <sstream>

bool XDecode::Send(const XAVPacket& packet) const {
    CHECK_CODEC_CTX_RET();
    FF_ERR_OUT(avcodec_send_packet(m_codec_ctx_,&packet),return {});
    return true;
}

bool XDecode::Send(const XAVPacket *packet) const {
    if (!packet){
        LOG_ERROR(GET_STR(packet is empey));
        return {};
    }
    return Send(*packet);
}

bool XDecode::Receive(XAVFrame &frame) const {

    CHECK_CODEC_CTX_RET();
    const auto avcodec_receive_res{avcodec_receive_frame(m_codec_ctx_,&frame)};
    if (AVERROR(EAGAIN) == avcodec_receive_res){
        return {};
    }
    FF_ERR_OUT(avcodec_receive_res,return {});

    if(m_codec_ctx_->hw_device_ctx) {
        XAVFrame hw_frame;
        FF_ERR_OUT(av_hwframe_transfer_data(&hw_frame,&frame,0), return {});
        frame = std::move(hw_frame);
    }
    return true;
}

bool XDecode::Receive(XAVFrame *frame) const {
    if (!frame){
        LOG_ERROR(GET_STR(packet is empey));
        return {};
    }
    return Receive(*frame);
}

XAVFrames XDecode::Flush() const {
    CHECK_CODEC_CTX_RET();
    FF_ERR_OUT(avcodec_send_packet(m_codec_ctx_,{}),return {});
    XAVFrames frames;
    while (true) {
        XAVFrame frame;
        FF_ERR_OUT(avcodec_receive_frame(m_codec_ctx_,&frame),return frames);
        if (m_codec_ctx_->hw_device_ctx) {
            XAVFrame hw_frame;
            FF_ERR_OUT(av_hwframe_transfer_data(&hw_frame,&frame,0), return frames);
            frame = std::move(hw_frame);
        }
        if (auto f{new_XAVFrame(frame)}){
            frames.emplace_back(std::move(f));
        }
    }
}

bool XDecode::InitHw(const int &type) {

    CHECK_CODEC_CTX_RET();
    if(avcodec_is_open(m_codec_ctx_)) {
        PRINT_ERR_TIPS(GET_STR(Please set before opening the decoder!));
        return {};
    }

    const auto hw_type{static_cast<AVHWDeviceType>(type)};
    AVBufferRef *hw_device_ctx{}; //硬件加速
    FF_ERR_OUT(av_hwdevice_ctx_create(&hw_device_ctx,hw_type,{},{},0),return {});
    m_codec_ctx_->hw_device_ctx = av_buffer_ref(hw_device_ctx);
    std::stringstream ss;
    ss << "Hardware Acceleration: " << av_hwdevice_get_type_name(hw_type);
    LOG_INFO(ss.str());
    return true;
}
