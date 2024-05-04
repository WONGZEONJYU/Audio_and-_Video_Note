//
// Created by Administrator on 2024/5/4.
//

#ifndef INC_17_AUDIO_MIX_AUDIOINFO_HPP
#define INC_17_AUDIO_MIX_AUDIOINFO_HPP

extern "C"{
#include <libavfilter/avfilter.h>
};

#include <string>
#include <memory>

class AudioInfo {
    void _swap(AudioInfo &) noexcept(true);
    void _move_construct_from(AudioInfo &&) noexcept(true);
    friend class Audio_Mix;
public:
    explicit AudioInfo(std::string &&,const int &,
                       const AVSampleFormat&,
                       const AVChannelLayout&) noexcept(true);
    AudioInfo(const AudioInfo&) = delete;
    AudioInfo& operator=(const AudioInfo&) = delete;
    AudioInfo(AudioInfo&&) noexcept(true);
    AudioInfo& operator=(AudioInfo&&) noexcept(true);

private:
    std::string m_name;
    int m_sample_rate{};
    AVSampleFormat m_sample_fmt{};
    AVChannelLayout m_ch_layout{};
    AVFilterContext *m_filter_ctx{};
};

using AudioInfo_sp_type = std::shared_ptr<AudioInfo>;

#endif //INC_17_AUDIO_MIX_AUDIOINFO_HPP
