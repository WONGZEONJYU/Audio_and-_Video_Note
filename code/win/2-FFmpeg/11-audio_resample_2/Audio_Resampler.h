#ifndef AUDIO_RESAMPLER_H
#define AUDIO_RESAMPLER_H


extern "C"{
#include <libswresample/swresample.h>
}

#include <atomic>
#include <memory>

class AVAudioFifo_t;
class SwrContext_t;

namespace rsmp
{
    struct Audio_Resampler_Params {
        // input params
        AVSampleFormat src_sample_fmt{};
        AVChannelLayout src_ch_layout{};
        int src_sample_rate{};
        // attribute_deprecated
        // uint64_t src_channel_layout{};

        // output params
        AVSampleFormat dst_sample_fmt{};
        AVChannelLayout dst_ch_layout{};
        int dst_sample_rate{};
        // attribute_deprecated
        // uint64_t dst_channel_layout{};

        explicit operator bool() const noexcept {
            return dst_sample_fmt == src_sample_fmt &&
                    src_ch_layout.u.mask == dst_ch_layout.u.mask &&
                    src_sample_rate == dst_sample_rate;
        }
    };

    class Audio_Resampler final{

        bool construct() noexcept;
        explicit Audio_Resampler(const Audio_Resampler_Params &);
        bool init_resampled_data();
        void destory_resampled_data();
        void av_opt_set_in() const;
        void av_opt_set_out() const;
        [[nodiscard]] AVFrame *alloc_out_frame(const int&) const;
        [[nodiscard]] int fifo_read_helper(uint8_t** , const int& , int64_t& );
        [[nodiscard]] int need_samples_num(const int&) const;

    public:
        using Audio_Resampler_t = std::shared_ptr<Audio_Resampler>;
        static Audio_Resampler_t create(const Audio_Resampler_Params & );
        Audio_Resampler(const Audio_Resampler&) = delete;
        Audio_Resampler& operator=(const Audio_Resampler&) = delete;
        ~Audio_Resampler();

        int send_frame(const AVFrame &);
        int send_frame(uint8_t **in_data,const int &in_nb_samples,const int64_t &pts);
        int send_frame(const uint8_t *in_data,const int& in_bytes, const int64_t &pts);
        AVFrame *receive_frame(const int &nb_samples);
        int receive_frame(uint8_t **out_data,const int &nb_samples, int64_t &pts);
        [[nodiscard]] int fifo_size() const;
        [[nodiscard]] int64_t start_pts() const;
        [[nodiscard]] int64_t cur_pts() const;
    private:

        const Audio_Resampler_Params m_Resampler_Params{};// 重采样的设置参数
        std::shared_ptr<AVAudioFifo_t> m_audio_fifo;
        std::shared_ptr<SwrContext_t> m_swr_ctx;

        std::atomic_bool m_is_fifo_only;    //不需要进行重采样,只需要缓存到 audio_fifo
        std::atomic_bool m_is_flushed;// flush的时候使用
        int64_t m_start_pts{AV_NOPTS_VALUE};          // 起始pts
        int64_t m_cur_pts{AV_NOPTS_VALUE};            // 当前pts
        uint8_t **m_resampled_data{};   // 用来缓存重采样后的数据
        uint64_t m_resampled_data_size{2048};    // 重采样后的采样数
        int m_src_channels{};           // 输入的通道数
        int m_dst_channels{};           // 输出通道数
        int64_t m_total_resampled_num{};    // 统计总共的重采样点数,目前只是统计

    };

}

#endif //AUDIO_RESAMPLER_H
