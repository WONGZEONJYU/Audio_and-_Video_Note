//
// Created by Administrator on 2024/4/11.
//

#include "AudioOutputStream.h"

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avio.h>
}

#if 0
AVFrame *get_audio_frame()
{
    auto frame{m_audio_ost.m_tmp_frame};

    auto q {reinterpret_cast<int16_t*>(frame->data[0])};

    /* check if we want to generate more frames */
    // 44100 * {1, 44100} = 1 --> 44100*5 * {1, 44100} = 5
    // 5 *{1,1} = 5
    if (av_compare_ts(m_audio_ost.m_next_pts, m_audio_ost.m_codecContext->time_base,
                      STREAM_DURATION, { 1, 1 }) >= 0) {
        return nullptr;
    }

    const auto nb_channels{m_audio_ost.m_codecContext->ch_layout.nb_channels};

    for (int j {}; j < frame->nb_samples; j++) {

        const auto v {static_cast<int16_t>(sin(m_audio_ost.m_t) * 10000)};

        for (int i {}; i < nb_channels; i++){
            *q++ = v;
        }

        m_audio_ost.m_t     += m_audio_ost.m_tincr;
        m_audio_ost.m_tincr += m_audio_ost.m_tincr2;
    }

    frame->pts = m_audio_ost.m_next_pts; // 使用samples作为计数 设置pts 0, nb_samples(1024) 2048
    m_audio_ost.m_next_pts += frame->nb_samples;    // 音频PTS使用采样点个数nb_samples叠加
    return frame;
}



#endif

