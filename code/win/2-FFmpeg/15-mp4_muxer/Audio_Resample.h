#ifndef AUDIO_RESAMPLE_H
#define AUDIO_RESAMPLE_H

#include <atomic>

#include "AVAudioFifo_t.h"
#include "SwrContext_t.h"
#include "ShareAVFrame.hpp"

struct Audio_Resample_Params {
    // input params
    explicit Audio_Resample_Params(const AVSampleFormat &src_sample_fmt = AV_SAMPLE_FMT_S16,
                                    const AVChannelLayout &src_ch_layout = AV_CHANNEL_LAYOUT_STEREO,
                                    const int &src_sample_rate = 44100,
                                    const AVSampleFormat &dst_sample_fmt = AV_SAMPLE_FMT_FLTP,
                                    const AVChannelLayout &dst_ch_layout = AV_CHANNEL_LAYOUT_STEREO,
                                    const int &dst_sample_rate = 44100) noexcept(true):
            m_src_sample_fmt(src_sample_fmt),
            m_dst_sample_fmt(dst_sample_fmt),
            m_src_ch_layout(src_ch_layout),
            m_dst_ch_layout(dst_ch_layout),
            m_src_sample_rate(src_sample_rate),
            m_dst_sample_rate(dst_sample_rate){}

    AVSampleFormat m_src_sample_fmt{},m_dst_sample_fmt{};
    AVChannelLayout m_src_ch_layout{},m_dst_ch_layout{};
    int m_src_sample_rate{},m_dst_sample_rate{};

    constexpr explicit operator bool() const noexcept(true){
        return m_src_sample_fmt == m_dst_sample_fmt &&
               m_src_ch_layout.u.mask == m_dst_ch_layout.u.mask &&
               m_src_sample_rate == m_dst_sample_rate;
    }
};

class Audio_Resample final{

    void Construct() noexcept(false);
    explicit Audio_Resample(const Audio_Resample_Params &) noexcept(true);
    void init_resampled_data() noexcept(false);
    void destroy_resample_data() noexcept(true);
    [[nodiscard]] ShareAVFrame_sp_type alloc_out_frame(const int&) const noexcept(false);
    int fifo_read_helper(uint8_t** , const int& , int64_t& ) noexcept(false);
    [[nodiscard]] int need_samples_num(const int&) const noexcept(true);
    void fill_audio_frame_helper(const uint8_t* , const int& ,AVFrame&) noexcept(false);
public:
    using Audio_Resample_t = std::shared_ptr<Audio_Resample>;

    static Audio_Resample_t create(const Audio_Resample_Params & ) noexcept(false);
    Audio_Resample(const Audio_Resample&) = delete;
    Audio_Resample& operator=(const Audio_Resample&) = delete;
    ~Audio_Resample();

    int send_frame(const AVFrame &) noexcept(false);
    int send_frame(uint8_t **in_data,const int &in_nb_samples,const int64_t &pts) noexcept(false);
    int send_frame(const uint8_t *in_data,const int& in_bytes, const int64_t &pts) noexcept(false);
    ShareAVFrame_sp_type receive_frame(const int &nb_samples = 0) noexcept(false);
    int receive_frame(uint8_t **out_data,const int &nb_samples, int64_t &pts) noexcept(false);

    [[nodiscard]] int fifo_size() const noexcept(true);

    [[nodiscard]] auto start_pts() const noexcept(true){
        return m_start_pts;
    }

    [[nodiscard]] int64_t cur_pts() const noexcept(true){
        return m_cur_pts;
    }

private:
    const Audio_Resample_Params m_Resample_Params{};// 重采样的设置参数
    AVAudioFifo_sp_type m_audio_fifo;
    SwrContext_sp_type m_swr_ctx;
    std::atomic_bool m_is_fifo_only; //不需要进行重采样,只需要缓存到 audio_fifo
    std::atomic_bool m_is_flushed;//flush的时候使用
    int64_t m_start_pts{AV_NOPTS_VALUE}; // 起始pts
    int64_t m_cur_pts{AV_NOPTS_VALUE}; // 当前pts
    uint8_t **m_resampled_data{};   // 用来缓存重采样后的数据
    int m_resampled_data_size {2048};    // 重采样后的采样数
    int64_t m_total_resampled_num{};    // 统计总共的重采样点数,目前只是统计
};

using Audio_Resample_type = typename Audio_Resample::Audio_Resample_t;

Audio_Resample_type new_Audio_Resample(const Audio_Resample_Params & ) noexcept(false);

#endif
