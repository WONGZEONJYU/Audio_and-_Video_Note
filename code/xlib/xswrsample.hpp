#ifndef SWRESAMPLE_HPP_
#define SWRESAMPLE_HPP_

#include "xavframe.hpp"
#include "xhelper.hpp"

struct XSwrParam {
    const AVChannelLayout *m_in_ch_layout{},
        *m_out_ch_layout{};
    int m_in_sample_fmt{},
        m_in_sample_rate{},
        m_out_sample_fmt{},
        m_out_sample_rate{};

    inline constexpr explicit operator bool() const {
        return m_in_ch_layout && m_out_ch_layout &&
            m_in_sample_fmt > 0 && m_in_sample_rate > 0 &&
                m_out_sample_fmt > 0 && m_out_sample_rate > 0;
    }

    inline constexpr bool operator !() const {
        return !operator bool();
    }

    [[nodiscard]] inline constexpr bool empty() const {
        return operator!();
    }
};

class XLIB_API XSwrSample final{

    static auto new_obj() -> XSwrSample_sp;
    explicit XSwrSample() = default;
    bool Construct() noexcept(true);
    bool Construct(const AVChannelLayout *out_ch_layout,
                   const int &out_sample_fmt,
                   const int &out_sample_rate,
                   const AVChannelLayout *in_ch_layout,
                   const int &in_sample_fmt,
                   const int &in_sample_rate,
                   const int &log_offset,
                   void *log_ctx) noexcept(true);
    bool Construct(const XSwrParam &,
        const int &log_offset,void *log_ctx) noexcept(true);

    void DeConstruct() noexcept(true);

    [[nodiscard]] int opt_set_ch_layout(const std::string& ,
                           const AVChannelLayout *) const noexcept(true);

    [[nodiscard]] int opt_set_sample_fmt (const std::string&,
                             const AVSampleFormat &) const noexcept(true);

    [[nodiscard]] int opt_set_sample_rate(const std::string&,
                             const int64_t &val) const noexcept(true);

public:
    /**
     * 调用前,请先设置好参数
     * @return true or false
     */
    [[nodiscard]] bool init() const noexcept(true);

    int convert(uint8_t **out,
                    const int &out_count,
                    const uint8_t * const *in,
                    const int &in_count ) const noexcept(true);

    /**
     * 输入的AVFrame和输出的AVFrame的信息必须要与配置本重采样器的参数一致
     * @param src
     * @param dst
     * @return ffmoeg error code or nb_samples
     */
    int convert(const XAVFrame &src, XAVFrame &dst) const noexcept(false);

    /**
    * compute destination number of samples
    * 计算目标样本数,内部重采样延迟缓冲区中可能会有剩余数据,
    * 本函数已经包含 src_samples + 剩余数据
     * @param src_samples
     * @return destination number of samples error at -1
     */
    [[nodiscard]] int64_t dst_num_samples(const int &src_samples = 1024) const;

    /**
     * 设置待重采样音频的通道数、采样格式、采样率
     * @return true or false
     */
    [[nodiscard]] bool set_input_ch_layout(const AVChannelLayout *)  noexcept(true);
    [[nodiscard]] bool set_input_sample_fmt(const int &) noexcept(true);
    [[nodiscard]] bool set_input_sample_rate(const int64_t &) noexcept(true);

    /**
     * 设置重采样后音频的通道数、采用格式、采样率
     * @return true or false
     */
    [[nodiscard]] bool set_output_ch_layout(const AVChannelLayout *) noexcept(true);
    [[nodiscard]] bool set_output_sample_fmt(const int &) noexcept(true);
    [[nodiscard]] bool set_output_sample_rate(const int64_t &) noexcept(true);

    [[nodiscard]] int64_t get_delay(const int64_t& ) const noexcept(true);
    [[nodiscard]] bool set_compensation(const int &sample_delta,
                           const int &compensation_distance) const noexcept(true);

private:
    SwrContext* m_swr_ctx_{};
    XSwrParam m_params_{};

public:
    friend XLIB_API XSwrSample_sp new_XSwrSample() noexcept(true);
    friend XLIB_API XSwrSample_sp new_XSwrSample(const XSwrParam &p,
                                    const int &log_offset,
                                    void *log_ctx) noexcept(true);
    friend XLIB_API XSwrSample_sp new_XSwrSample(const AVChannelLayout *out_ch_layout,
                                        const int &out_sample_fmt,
                                        const int &out_sample_rate,
                                        const AVChannelLayout *in_ch_layout,
                                        const int &in_sample_fmt,
                                        const int &in_sample_rate,
                                        const int &log_offset ,
                                        void *log_ctx) noexcept(true);
    ~XSwrSample();
    X_DISABLE_COPY_MOVE(XSwrSample)
};

XLIB_API XSwrSample_sp new_XSwrSample() noexcept(true);
XLIB_API XSwrSample_sp new_XSwrSample(const XSwrParam &p,
                                    const int &log_offset,
                                    void *log_ctx) noexcept(true);
XLIB_API XSwrSample_sp new_XSwrSample(const AVChannelLayout *out_ch_layout,
                                    const int &out_sample_fmt,
                                    const int &out_sample_rate,
                                    const AVChannelLayout *in_ch_layout,
                                    const int &in_sample_fmt,
                                    const int &in_sample_rate,
                                    const int &log_offset = {},
                                    void *log_ctx = {}) noexcept(true);


#endif
