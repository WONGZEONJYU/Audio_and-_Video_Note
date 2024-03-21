#ifndef AUDIO_RESAMPLER_H
#define AUDIO_RESAMPLER_H

#include <memory>
#include <atomic>

extern "C" {
#include <libavutil/audio_fifo.h>
#include <libswresample/swresample.h>

}

namespace rsmp
{
    struct Audio_Resampler_Params {
        // input params
        AVSampleFormat src_sample_fmt;
        AVChannelLayout src_ch_layout;
        int src_sample_rate;
        uint64_t src_channel_layout;

        // output params
        AVSampleFormat dst_sample_fmt;
        AVChannelLayout dst_ch_layout;
        int dst_sample_rate;
        uint64_t dst_channel_layout;
    };

    class Audio_Resampler {
        bool init();

    public:
        Audio_Resampler(const Audio_Resampler&) = delete;
        Audio_Resampler& operator=(const Audio_Resampler&) = delete;
        explicit Audio_Resampler() = default;
        explicit Audio_Resampler(const Audio_Resampler_Params &);

    private:
        Audio_Resampler_Params m_Resampler_Params{};// 重采样的设置参数
        std::shared_ptr<SwrContext> m_swr_ctx;
        std::shared_ptr<AVAudioFifo> m_audio_fifo;
        std::atomic_bool m_is_fifo_only;
        std::atomic_bool m_is_flushed;// flush的时候使用
        int64_t m_start_pts{};          // 起始pts
        int64_t m_cur_pts{};            // 当前pts
        uint8_t **m_resampled_data{};   // 用来缓存重采样后的数据
        int m_resampled_data_size{};    // 重采样后的采样数
        int m_src_channels{};           // 输入的通道数
        int m_dst_channels{};           // 输出通道数
        int64_t total_resampled_num{};    // 统计总共的重采样点数,目前只是统计

    };
}





#endif //AUDIO_RESAMPLER_H
