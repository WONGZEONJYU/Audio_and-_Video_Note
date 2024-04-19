//
// Created by Administrator on 2024/4/19.
//

extern "C"{
#include <libavcodec/avcodec.h>
}

#include "ShareAVPacket.hpp"

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

void ShareAVPacket::Construct() noexcept(false) {
    m_packet = av_packet_alloc();
    if (!m_packet){
        throw std::runtime_error("av_packet_alloc failed\n");
    }
}

void ShareAVPacket::DeConstruct() noexcept(true) {
    av_packet_free(&m_packet);
}

ShareAVPacket::~ShareAVPacket() {
    DeConstruct();
}
