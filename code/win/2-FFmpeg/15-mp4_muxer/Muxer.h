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

class Muxer {

    explicit Muxer(const char*);
    void construct() noexcept(false);
public:
    using Muxer_sp_type = std::shared_ptr<Muxer>;
    Muxer(const Muxer&) = delete;
    Muxer& operator=(const Muxer&) = delete;

private:
    AVFormatContext *m_fmt_ctx{};
    AVCodecContext *m_codec_ctx{};
    AVStream* m_video_stream{},*m_audio_stream{};
    int m_video_index{-1},m_audio_index{-1};
    std::string m_url;

};


#endif //INC_15_MP4_MUXER_MUXER_H
