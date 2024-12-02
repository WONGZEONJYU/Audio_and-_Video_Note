extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

#include "xencode.hpp"
#include "xavframe.hpp"
#include "xavpacket.hpp"

XAVPacket_sp XEncode::Encode(const XAVFrame &frame) const{

    CHECK_CODEC_CTX_RET();

    if (const auto ret{avcodec_send_frame(m_codec_ctx_,&frame)};
        0 != ret || AVERROR(EAGAIN) != ret){
        FF_ERR_OUT(ret,return {});
    }

    XAVPacket_sp packet;
    IS_SMART_NULLPTR(packet = new_XAVPacket(),return {});
    const auto receive_packet_res{avcodec_receive_packet(m_codec_ctx_,packet.get())};
    if (AVERROR(EAGAIN) == receive_packet_res){
        return {};
    }
    FF_ERR_OUT(receive_packet_res,return {});
    return packet;
}

XAVPackets XEncode::Flush() const{
    CHECK_CODEC_CTX_RET();
    FF_ERR_OUT(avcodec_send_frame(m_codec_ctx_,{}),return {});
    XAVPackets packets;
    while (true){
        XAVPacket_sp packet;
        IS_SMART_NULLPTR(packet = new_XAVPacket(),return packets);
        FF_ERR_OUT(avcodec_receive_packet(m_codec_ctx_,packet.get()),return packets);
        packets.push_back(std::move(packet));
    }
}
