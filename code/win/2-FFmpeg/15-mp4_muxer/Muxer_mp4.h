//
// Created by Administrator on 2024/4/16.
//

#ifndef INC_15_MP4_MUXER_MUXER_MP4_H
#define INC_15_MP4_MUXER_MUXER_MP4_H

#include <fstream>
#include <vector>
#include <memory>
#include <memory_resource>

struct AVPacket;

class Muxer_mp4 {
    static inline constexpr auto PACKETS_SIZE {100*100 *1024};
    explicit Muxer_mp4();
    void Construct() noexcept(false);
    void DeConstruct() noexcept(true);
public:
    using Muxer_mp4_sp_type = std::shared_ptr<Muxer_mp4>;
    Muxer_mp4(const Muxer_mp4&) = delete;
    Muxer_mp4& operator=(const Muxer_mp4&) = delete;
    static Muxer_mp4_sp_type create() noexcept(false);
    ~Muxer_mp4();
private:
    std::pmr::monotonic_buffer_resource resource;
    std::pmr::vector<AVPacket> m_packets;
    std::pmr::unsynchronized_pool_resource m_mem_pool;
    std::ifstream m_yuv_file;
    std::ifstream m_pcm_file;
    void *m_yuv_buffer{},*m_pcm_buffer{};
};

using Muxer_mp4_sp_type = typename Muxer_mp4::Muxer_mp4_sp_type;

#endif //INC_15_MP4_MUXER_MUXER_MP4_H
