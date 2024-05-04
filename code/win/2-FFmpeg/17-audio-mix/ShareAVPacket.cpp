//
// Created by Administrator on 2024/4/19.
//
extern "C"{
#include <libavcodec/avcodec.h>
}

#include <string>
#include "ShareAVPacket.hpp"

ShareAVPacket::ShareAVPacket()noexcept(true):
m_packet(av_packet_alloc()) {

}

void ShareAVPacket::Construct() const noexcept(false) {
    if (!m_packet){
        throw std::runtime_error("av_packet_alloc failed\n");
    }
}

ShareAVPacket::ShareAVPacket_sp_type ShareAVPacket::create() noexcept(false) {
    ShareAVPacket_sp_type obj;
    try {
        obj = std::move(ShareAVPacket_sp_type(new ShareAVPacket));
    } catch (const std::bad_alloc &e) {
        throw std::runtime_error("new ShareAVPacket  failed: " + std::string (e.what()) + "\n");
    }

    try {
        obj->Construct();
        return obj;
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw std::runtime_error("ShareAVPacket Construct failed: " + std::string (e.what()) + "\n");
    }
}

void ShareAVPacket::DeConstruct() noexcept(true) {
    av_packet_free(const_cast<AVPacket**>(&m_packet));
}

ShareAVPacket::~ShareAVPacket() {
    DeConstruct();
}

ShareAVPacket_sp_type new_ShareAVPacket() noexcept(false)
{
    return ShareAVPacket::create();
}
