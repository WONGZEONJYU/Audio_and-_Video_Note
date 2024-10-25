#ifndef SWRESAMPLE_HPP_
#define SWRESAMPLE_HPP_

#include "xhelper.hpp"

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
                    const int &in_count ) const noexcept(false);

    /**
     * 设置待重采样音频的通道数、采样格式、采样率
     * @return true or false
     */
    [[nodiscard]] bool set_input_ch_layout(const AVChannelLayout *) const noexcept(true);
    [[nodiscard]] bool set_input_sample_fmt(const int &) const noexcept(true);
    [[nodiscard]] bool set_input_sample_rate(const int64_t &) const noexcept(true);

    /**
     * 设置重采样后音频的通道数、采用格式、采样率
     * @return true or false
     */
    [[nodiscard]] bool set_output_ch_layout(const AVChannelLayout *) const noexcept(true);
    [[nodiscard]] bool set_output_sample_fmt(const int &) const noexcept(true);
    [[nodiscard]] bool set_output_sample_rate(const int64_t &) const noexcept(true);

    [[nodiscard]] int64_t get_delay(const int64_t& ) const noexcept(true);
    [[nodiscard]] bool set_compensation(const int &sample_delta,
                           const int &compensation_distance) const noexcept(true);

private:
    SwrContext* m_swr_ctx_{};

public:
    friend XLIB_API XSwrSample_sp new_XSwrSample() noexcept(true);
    friend XLIB_API XSwrSample_sp new_XSwrSample(const AVChannelLayout *out_ch_layout,
                                        const int &out_sample_fmt,
                                        const int &out_sample_rate,
                                        const AVChannelLayout *in_ch_layout,
                                        const int &in_sample_fmt,
                                        const int &in_sample_rate,
                                        const int &log_offset ,
                                        void *log_ctx) noexcept(true);
    ~XSwrSample();
    X_DISABLE_COPY(XSwrSample)
};

XLIB_API XSwrSample_sp new_XSwrSample() noexcept(true);
XLIB_API XSwrSample_sp new_XSwrSample(const AVChannelLayout *out_ch_layout,
                                    const int &out_sample_fmt,
                                    const int &out_sample_rate,
                                    const AVChannelLayout *in_ch_layout,
                                    const int &in_sample_fmt,
                                    const int &in_sample_rate,
                                    const int &log_offset = {},
                                    void *log_ctx = {}) noexcept(true);

#endif
