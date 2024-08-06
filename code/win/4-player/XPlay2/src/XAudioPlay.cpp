//
// Created by Administrator on 2024/8/6.
//

#include "XAudioPlay.hpp"

void XAudioPlay::set_Audio_parameter(const int &SampleRate,
                                      const int &Channels,
                                      const int &SampleFormat) noexcept(true) {
    m_SampleRate = SampleRate;
    m_SampleFormat = SampleFormat;
    m_Channels = Channels;
}
