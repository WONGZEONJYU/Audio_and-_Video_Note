extern "C" {
#include <libavutil/avutil.h>
}

#include "xavpacket.hpp"

XAVPacket::XAVPacket() : AVPacket() {
    av_packet_unref(this);
}

XAVPacket::XAVPacket(const AVPacket &packet) : AVPacket() {
    av_packet_ref(this,std::addressof(packet));
}

XAVPacket::XAVPacket(const AVPacket *packet) : AVPacket() {
    av_packet_ref(this,packet);
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

bool XAVPacket::Make_Writable() {
    FF_ERR_OUT(av_packet_make_writable(this),return {});
    return true;
}

void XAVPacket::Reset(const AVPacket *packet) {
    av_packet_unref(this);
    if (packet) {
        av_packet_ref(this,packet);
    }
}

void XAVPacket::Ref_fromAVPacket(const AVPacket *packet) {
    if (packet) {
        av_packet_unref(this);
        av_packet_ref(this,packet);
    }
}

void XAVPacket::Ref_fromAVPacket(const AVPacket &packet) {
    Ref_fromAVPacket(std::addressof(packet));
}

void XAVPacket::Move_FromAVPacket(AVPacket *packet) {
    if (packet) {
        av_packet_unref(this);
        av_packet_move_ref(this,packet);
    }
}

void XAVPacket::Move_FromAVPacket(AVPacket &&packet) {
    Move_FromAVPacket(std::addressof(packet));
}

XAVPacket_sp new_XAVPacket() noexcept(true) {
    XAVPacket_sp obj;
    TRY_CATCH(CHECK_EXC(obj = std::make_shared<XAVPacket>()),return {});
    return obj;
}

XAVPacket_sp new_XAVPacket(const AVPacket &packet) noexcept(true) {
    XAVPacket_sp obj;
    TRY_CATCH(CHECK_EXC(obj = std::make_shared<XAVPacket>(packet)),return {});
    return obj;
}

XAVPacket_sp new_XAVPacket(const AVPacket *packet) noexcept(true) {
    XAVPacket_sp obj;
    TRY_CATCH(CHECK_EXC(obj = std::make_shared<XAVPacket>(packet)),return {});
    return obj;
}
