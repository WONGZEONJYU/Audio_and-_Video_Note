//
// Created by Administrator on 2024/7/29.
//

extern "C" {
#include <libavutil/avutil.h>
}

#include "XAVPacket.hpp"
#include "XHelper.hpp"

XAVPacket::XAVPacket() : AVPacket() {
    av_packet_unref(this);
}

XAVPacket::XAVPacket(const XAVPacket &obj) : XAVPacket(){
    av_packet_ref(this,std::addressof(obj));
}

XAVPacket::XAVPacket(XAVPacket &&obj) noexcept : XAVPacket() {
    av_packet_move_ref(this,std::addressof(obj));
}

XAVPacket &XAVPacket::operator=(const XAVPacket &obj) {
    auto obj_{std::addressof(obj)};
    if (this != obj_){
        av_packet_unref(this);//先释放自身,再调用av_packet_ref
        av_packet_ref(this,obj_);
    }
    return *this;
}

XAVPacket &XAVPacket::operator=(XAVPacket &&obj) noexcept {
    auto obj_{std::addressof(obj)};
    if (this != obj_){
        av_packet_unref(this);//先释放自身,再调用av_packet_move_ref
        av_packet_move_ref(this,obj_);
    }
    return *this;
}

XAVPacket::~XAVPacket() {
    av_packet_unref(this);
}

XAVPacket_sptr new_XAVPacket() noexcept(false) {
    XAVPacket_sptr obj;
    CHECK_EXC(obj = std::make_shared<XAVPacket>());
    return obj;
}
