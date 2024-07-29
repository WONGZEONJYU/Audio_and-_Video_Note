//
// Created by Administrator on 2024/7/29.
//

extern "C" {
#include <libavutil/avutil.h>
}

#include "XAVPacket.hpp"
#include <string>

XAVPacket::XAVPacket() : AVPacket() {

    pts             = AV_NOPTS_VALUE;
    dts             = AV_NOPTS_VALUE;
    pos             = -1;
    time_base       = {0,1};
}

XAVPacket::XAVPacket(const XAVPacket &obj) : XAVPacket(){
    av_packet_ref(this,std::addressof(obj));
}

XAVPacket::XAVPacket(XAVPacket &&obj) noexcept : XAVPacket() {
    av_packet_move_ref(this,std::addressof(obj));
}

XAVPacket &XAVPacket::operator=(const XAVPacket &obj) {
    if (this != std::addressof(obj)){
        av_packet_ref(this,std::addressof(obj));
    }
    return *this;
}

XAVPacket &XAVPacket::operator=(XAVPacket &&obj) noexcept {
    if (this != std::addressof(obj)){
        av_packet_move_ref(this,std::addressof(obj));
    }
    return *this;
}

XAVPacket::~XAVPacket() {
    av_packet_unref(this);
}

XAVPacket_sptr new_XAVPacket() noexcept(false)
{
    try {
        return std::make_shared<XAVPacket>();
    } catch (...) {
        throw std::runtime_error(std::string(__func__ ) + " error!\n");
    }
}
