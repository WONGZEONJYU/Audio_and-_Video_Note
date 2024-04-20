//
// Created by Administrator on 2024/4/15.
//

#ifndef INC_15_MP4_MUXER_MUXER_H
#define INC_15_MP4_MUXER_MUXER_H

#include <string>
#include <memory>


struct AVFormatContext;
struct AVCodecContext;
struct AVStream;

#include "ShareAVPacket.hpp"

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
    void Send_header() const noexcept(false);
    void Send_packet(const ShareAVPacket_sp_type& ,const AVRational &,const AVRational &) const noexcept(false);
    void Send_trailer() const noexcept(false);
    [[nodiscard]] AVStream *create_stream() const noexcept(false);
    [[nodiscard]] std::string url() const noexcept(true){
        return m_url;
    }
    void dump_format(const int& ) const noexcept(true);
private:
    const std::string m_url;
    AVFormatContext *m_fmt_ctx{};
};

using Muxer_sp_type = typename Muxer::Muxer_sp_type ;

#endif //INC_15_MP4_MUXER_MUXER_H
