//
// Created by Administrator on 2024/5/4.
//

#include "AudioInfo.hpp"

void AudioInfo::_swap(AudioInfo& r) noexcept(true)
{
    std::swap(m_name,r.m_name);
    std::swap(m_sample_rate,r.m_sample_rate);
    std::swap(m_sample_fmt,r.m_sample_fmt);
    std::swap(m_ch_layout,r.m_ch_layout);
    std::swap(m_filter_ctx,r.m_filter_ctx);
}

void AudioInfo::_move_construct_from(AudioInfo && r) noexcept(true)
{
    _swap(r);
}

AudioInfo::AudioInfo(std::string &&name,const int &sample_rate,
                     const AVSampleFormat &sample_fmt,const AVChannelLayout &ch_layout)noexcept(true):
        m_name(std::move(name)),
        m_sample_rate(sample_rate),
        m_sample_fmt(sample_fmt),
        m_ch_layout(ch_layout)
{
}

AudioInfo::AudioInfo(AudioInfo &&r) noexcept(true)
{
    _move_construct_from(std::move(r));
}

AudioInfo& AudioInfo::operator=(AudioInfo &&r) noexcept(true)
{
    AudioInfo(std::move(r))._swap(*this);
    return *this;
}
