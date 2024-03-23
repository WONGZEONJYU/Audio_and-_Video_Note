#ifndef AUDIO_RESAMPLER_H
#define AUDIO_RESAMPLER_H

#include <atomic>
#include <memory>

extern "C" {
#include <libavutil/audio_fifo.h>
#include <libswresample/swresample.h>
#include <libavutil/error.h>
}

namespace rsmp
{
    static std::string av_get_err(const int& errnum) {
        constexpr auto ERROR_STRING_SIZE {1024};
        char err_buf[ERROR_STRING_SIZE]{};
        av_strerror(errnum, err_buf, std::size(err_buf));
        return {err_buf};
    }

    struct Audio_Resampler_Params {
        // input params
        AVSampleFormat src_sample_fmt{};
        AVChannelLayout src_ch_layout{};
        int src_sample_rate{};
        attribute_deprecated
        uint64_t src_channel_layout{};

        // output params
        AVSampleFormat dst_sample_fmt{};
        AVChannelLayout dst_ch_layout{};
        int dst_sample_rate{};
        attribute_deprecated
        uint64_t dst_channel_layout{};

        explicit operator bool() const noexcept{
            return dst_sample_fmt == src_sample_fmt &&
        src_ch_layout.u.mask == dst_ch_layout.u.mask &&
        src_sample_rate == dst_sample_rate;
        }
    };

    class Audio_Resampler final{

        struct AVAudioFifo_t final{
            AVAudioFifo* m_audio_fifo{};

            AVAudioFifo_t(const AVAudioFifo_t&) = delete;
            AVAudioFifo_t& operator=(const AVAudioFifo_t&) = delete;

            explicit AVAudioFifo_t(const AVSampleFormat &sample_fmt,
                const int& channels,
                const int &nb_samples);

            ~AVAudioFifo_t();
        };

        struct SwrContext_t final{
            SwrContext* m_swr_ctx{};
            SwrContext_t(const SwrContext_t&) = delete;
            SwrContext_t& operator=(const SwrContext_t&) = delete;
            explicit SwrContext_t();
            ~SwrContext_t();
        };

        bool construct() noexcept;
        explicit Audio_Resampler(const Audio_Resampler_Params &);
        int init_resampled_data();
        void destory_resampled_data();
        void av_opt_set_in() const;
        void av_opt_set_out() const;

    public:
        using Audio_Resampler_t = std::shared_ptr<Audio_Resampler>;
        static Audio_Resampler_t NewInstance(const Audio_Resampler_Params & );
        Audio_Resampler(const Audio_Resampler&) = delete;
        Audio_Resampler& operator=(const Audio_Resampler&) = delete;
        ~Audio_Resampler();
    private:

        std::shared_ptr<AVAudioFifo_t> m_audio_fifo;
        std::shared_ptr<SwrContext_t> m_swr_ctx;

        Audio_Resampler_Params m_Resampler_Params{};// 重采样的设置参数
        std::atomic_bool m_is_fifo_only;    //不需要进行重采样,只需要缓存到 audio_fifo
        std::atomic_bool m_is_flushed;// flush的时候使用
        int64_t m_start_pts{AV_NOPTS_VALUE};          // 起始pts
        int64_t m_cur_pts{AV_NOPTS_VALUE};            // 当前pts
        uint8_t **m_resampled_data{};   // 用来缓存重采样后的数据
        uint64_t m_resampled_data_size{2048};    // 重采样后的采样数
        uint64_t m_src_channels{};           // 输入的通道数
        uint64_t m_dst_channels{};           // 输出通道数
        int64_t total_resampled_num{};    // 统计总共的重采样点数,目前只是统计

    };

}

#endif //AUDIO_RESAMPLER_H
