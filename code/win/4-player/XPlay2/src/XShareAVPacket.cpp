//
// Created by Administrator on 2024/4/19.
//
extern "C"{
#include <libavcodec/avcodec.h>
}

#include <string>
#include "XShareAVPacket.hpp"

XShareAVPacket::XShareAVPacket()noexcept(true):
m_packet(av_packet_alloc()) {

}

void XShareAVPacket::Construct() const noexcept(false) {
    if (!m_packet){
        throw std::runtime_error("av_packet_alloc failed\n");
    }
}

XShareAVPacket::ShareAVPacket_sp_type XShareAVPacket::create() noexcept(false) {
    ShareAVPacket_sp_type obj;
    try {
        obj.reset(new XShareAVPacket);
        obj->Construct();
        return obj;
    } catch (const std::bad_alloc &e) {
        throw std::runtime_error("new XShareAVPacket  failed: " + std::string (e.what()) + "\n");
    }catch (const std::runtime_error &e) {
        obj.reset();
        throw std::runtime_error("XShareAVPacket Construct failed: " + std::string (e.what()) + "\n");
    }
}

void XShareAVPacket::DeConstruct() noexcept(true) {
    av_packet_free(&m_packet);
}

XShareAVPacket::~XShareAVPacket() {
    DeConstruct();
}

ShareAVPacket_sp_type new_ShareAVPacket() noexcept(false)
{
    return XShareAVPacket::create();
}
