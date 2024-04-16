//
// Created by Administrator on 2024/4/15.
//
extern "C"
{
#include <libavformat/avformat.h>
}

#include "Muxer.h"
#include "AVHelper.h"

Muxer::Muxer(const char *url) {

}

void Muxer::Construct() noexcept(false) {

}

void Muxer::DeConstruct() noexcept {
    if (m_fmt_ctx){
        avformat_close_input(&m_fmt_ctx);
    }

}

Muxer::~Muxer() {
    DeConstruct();
}

void Muxer::Send_header() {

}

void Muxer::Send_packet() noexcept(false) {

}

void Muxer::Send_trailer() noexcept(false) {

}




