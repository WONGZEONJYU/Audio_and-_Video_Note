//
// Created by Administrator on 2024/4/16.
//

#ifndef INC_15_MP4_MUXER_MUXER_MP4_H
#define INC_15_MP4_MUXER_MUXER_MP4_H

extern "C"{
#include <libavformat/avformat.h>
}

#include <fstream>
#include <vector>
#include <memory>
#include <memory_resource>

#include "AudioOutputStream.hpp"
#include "VideoOutputStream.h"
#include "Muxer.h"

class Muxer_mp4 {
    static inline constexpr auto PACKETS_SIZE {100*100*1024};
    static inline constexpr auto YUV_WIDTH{720},YUV_HEIGHT{576};
    static inline constexpr auto YUV_FPS {25};
    static inline constexpr auto TIME_BASE{(AVRational){1, AV_TIME_BASE}};
    static inline constexpr auto YUV_FMT{AV_PIX_FMT_YUV420P};
    static inline constexpr auto PCM_FMT{AV_SAMPLE_FMT_S16};
    static inline constexpr auto CHANNEL_LAYOUT {AVChannelLayout(AV_CHANNEL_LAYOUT_STEREO)};
    static inline constexpr auto PCM_SAMPLE_RATE {44100};
    static inline constexpr auto AUDIO_BIT_RATE{128*1024};
    static inline constexpr auto VIDEO_BIT_RATE {500*1024};

    explicit Muxer_mp4(const std::string &,const std::string &) noexcept(true);
    void Construct(std::string &&) noexcept(false);
    void DeConstruct() noexcept(true);
public:
    using Muxer_mp4_sp_type = std::shared_ptr<Muxer_mp4>;
    Muxer_mp4(const Muxer_mp4&) = delete;
    Muxer_mp4& operator=(const Muxer_mp4&) = delete;
    static Muxer_mp4_sp_type create(const std::string &yuv_file_name,
                                    const std::string &pcm_file_name,
                                    std::string&& url) noexcept(false);
    ~Muxer_mp4();

private:
    std::ifstream m_yuv_file;
    std::ifstream m_pcm_file;
    std::pmr::monotonic_buffer_resource resource;
    std::pmr::vector<ShareAVPacket_sp_type> m_packets;
    std::pmr::unsynchronized_pool_resource m_mem_pool;
    Muxer_sp_type m_muxer;
    AudioOutputStream_sp_type m_AudioOutputStream;
    VideoOutputStream_sp_type m_VideoOutputStream;
    uint8_t *m_yuv_buffer{},*m_pcm_buffer{};
    uint64_t m_yuv_buffer_size{},m_pcm_buffer_size{};
    int64_t video_pts{},audio_pts{};
};

using Muxer_mp4_sp_type = typename Muxer_mp4::Muxer_mp4_sp_type;

Muxer_mp4_sp_type new_Muxer_mp4(const std::string &yuv_file_name,
                                const std::string &pcm_file_name,
                                std::string &&out_file) noexcept(false);

#endif
