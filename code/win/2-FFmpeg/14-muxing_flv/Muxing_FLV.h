#ifndef INC_14_MUXING_FLV_MUXING_FLV_H
#define INC_14_MUXING_FLV_MUXING_FLV_H

#include <memory>
#include <string>

extern "C"{
#include <libavutil/pixfmt.h>
};

#include "VideoOutputStream.h"
#include "AudioOutputStream.h"

class Muxing_FLV final {

    explicit Muxing_FLV(std::string );
    bool construct() noexcept;
    bool alloc_fmt_ctx();
    bool open_media_file();
    bool write_header();
    bool write_trailer();
public:
    using Muxing_FLV_sp_type = std::shared_ptr<Muxing_FLV>;
    Muxing_FLV(const Muxing_FLV&) = delete;
    Muxing_FLV& operator= (const Muxing_FLV&) = delete;
    static Muxing_FLV_sp_type create(const std::string &) noexcept(false);
    ~Muxing_FLV();
    void exec() noexcept;

private:
    const std::string m_filename;
    AVFormatContext *m_fmt_ctx{};
    std::shared_ptr<OutputStreamAbstract> video_output_stream,
    audio_output_stream;
};

#endif
