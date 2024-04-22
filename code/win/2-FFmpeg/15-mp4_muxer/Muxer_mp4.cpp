//
// Created by Administrator on 2024/4/16.
//

extern "C"{
#include <libavformat/avformat.h>
}

#include "Muxer_mp4.h"

Muxer_mp4::Muxer_mp4(): resource(PACKETS_SIZE),m_packets(&resource){

}

void Muxer_mp4::Construct() noexcept(false) {

}

Muxer_mp4::Muxer_mp4_sp_type Muxer_mp4::create() noexcept(false) {

    return {};
}

Muxer_mp4::~Muxer_mp4() {
    DeConstruct();
}

void Muxer_mp4::DeConstruct() noexcept(true) {
    m_mem_pool.deallocate(m_yuv_buffer,0);
    m_mem_pool.deallocate(m_pcm_buffer,0);
}

