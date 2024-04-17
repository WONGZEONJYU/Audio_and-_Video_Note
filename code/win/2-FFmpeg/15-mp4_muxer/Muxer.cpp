//
// Created by Administrator on 2024/4/15.
//
extern "C" {
#include <libavformat/avformat.h>
}

#include "Muxer.h"
#include "AVHelper.h"

Muxer::Muxer_sp_type Muxer::create(std::string &&url) {

    Muxer_sp_type obj;
    try {
        obj = Muxer_sp_type(new Muxer(std::move(url)));
    } catch (const std::bad_alloc &e) {
        throw std::runtime_error("new Muxer failed\n");
    }

    try {
        obj->Construct();
        return obj;
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw std::runtime_error("Muxer Construct failed: " + std::string(e.what()) + "\n");
    }
}

Muxer::Muxer(std::string &&url):m_url(std::move(url)) {

}

void Muxer::Construct() noexcept(false) {

}

void Muxer::open() noexcept(false) {

}


void Muxer::Send_header() {

}

void Muxer::Send_packet() noexcept(false) {

}

void Muxer::Send_trailer() noexcept(false) {

}

void Muxer::DeConstruct() noexcept {
    if (m_fmt_ctx){
        avio_close(m_fmt_ctx->pb);
        avformat_close_input(&m_fmt_ctx);
    }
}

Muxer::~Muxer() {
    DeConstruct();
}

