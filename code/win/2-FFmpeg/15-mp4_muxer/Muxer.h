//
// Created by Administrator on 2024/4/15.
//

#ifndef INC_15_MP4_MUXER_MUXER_H
#define INC_15_MP4_MUXER_MUXER_H

#include <string>
#include <memory>

#include "MuxerAbstract.h"

struct AVFormatContext;
struct AVCodecContext;
struct AVStream;

class Muxer {

    explicit Muxer(std::string &&);
    void Construct() noexcept(false);
    void DeConstruct() noexcept(true);
    void Alloc_FormatContext() noexcept(false);
    void open() noexcept(false);

public:
    using Muxer_sp_type = std::shared_ptr<Muxer>;
    static Muxer_sp_type create(std::string &&) noexcept(false);
    ~Muxer();
    void Send_header();
    void Send_packet(AVPacket *,const AVRational &,const AVRational &) noexcept(false);
    void Send_trailer() noexcept(false);
    AVStream *create_stream() noexcept(false);

private:
    const std::string m_url;
    AVFormatContext *m_fmt_ctx{};
};

#endif //INC_15_MP4_MUXER_MUXER_H
