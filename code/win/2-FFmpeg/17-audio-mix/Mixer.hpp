//
// Created by wong on 2024/5/4.
//

#ifndef INC_17_AUDIO_MIX_MIXER_HPP
#define INC_17_AUDIO_MIX_MIXER_HPP

#include "Audio_Mix.hpp"
#include <memory_resource>

class Mixer final {
    static inline constexpr auto FMT_0{AV_SAMPLE_FMT_FLTP},
                                FMT_1{AV_SAMPLE_FMT_S16};
    static inline constexpr AVChannelLayout CH_0 AV_CHANNEL_LAYOUT_STEREO,
    CH_1 = CH_0;

    explicit Mixer(const char **) noexcept(true);
    void Construct() noexcept(false);
    void DeConstruct() noexcept(true);

public:
    using Mixer_sp_type = std::shared_ptr<Mixer>;
    static Mixer_sp_type create(const char **);
    ~Mixer();
    void exec() noexcept(false);

private:
    std::ifstream m_input_file_1,m_input_file_2;
    std::ofstream m_output_file;
    std::pmr::unsynchronized_pool_resource m_mem_pool;
    size_t m_pcm_flt_buf_size{},m_pcm_s16_buf_size{},m_out_buf_size{};
    uint8_t *m_pcm_flt_buf{},*m_pcm_s16_buf{},*m_out_buf{};
};

using Mixer_sp_type = Mixer::Mixer_sp_type;
Mixer_sp_type new_Mixer(const char **) noexcept(false);

#endif //INC_17_AUDIO_MIX_MIXER_HPP
