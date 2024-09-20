
#ifndef SWRCONTEXT_T_H
#define SWRCONTEXT_T_H

#include <memory>
#include "xhelper.hpp"

enum AVSampleFormat;
struct AVChannelLayout;
struct SwrContext;
class XSwrContext ;
using SwrContext_sptr = std::shared_ptr<XSwrContext>;

class XSwrContext final{

    static XSwrContext* new_XSwrContext() noexcept(false);
    explicit XSwrContext() = default;
    void Construct() noexcept(false);
    void Construct(const AVChannelLayout *out_ch_layout,
                   const AVSampleFormat &out_sample_fmt,
                   const int &out_sample_rate,
                   const AVChannelLayout *in_ch_layout,
                   const AVSampleFormat &in_sample_fmt,
                   const int &in_sample_rate,
                   const int &log_offset,
                   void *log_ctx ) noexcept(false);

    void DeConstruct() noexcept(true);

    [[nodiscard]] int opt_set_ch_layout(const std::string& ,
                           const AVChannelLayout *) const noexcept(true);

    [[nodiscard]] int opt_set_sample_fmt (const std::string&,
                             const AVSampleFormat &) const noexcept(true);

    [[nodiscard]] int opt_set_sample_rate(const std::string&,
                             const int64_t &val) const noexcept(true);

public:
    static SwrContext_sptr create() noexcept(false);
    static SwrContext_sptr create(const AVChannelLayout *out_ch_layout,
                                  const AVSampleFormat &out_sample_fmt,
                                  const int &out_sample_rate,
                                  const AVChannelLayout *in_ch_layout,
                                  const AVSampleFormat &in_sample_fmt,
                                  const int &in_sample_rate,
                                  const int &log_offset = 0,
                                  void *log_ctx  = nullptr) noexcept(false);


    void init() const noexcept(false);

    int convert (uint8_t **out,
                    const int &out_count,
                    const uint8_t * const *in,
                    const int &in_count ) const noexcept(false);

    void set_input_ch_layout(const AVChannelLayout *) const noexcept(false);
    void set_input_sample_fmt(const AVSampleFormat &) const noexcept(false);
    void set_input_sample_rate(const int64_t &) const noexcept(false);

    void set_output_ch_layout(const AVChannelLayout *) const noexcept(false);
    void set_output_sample_fmt(const AVSampleFormat &) const noexcept(false);
    void set_output_sample_rate(const int64_t &) const noexcept(false);

    [[nodiscard]] int64_t get_delay(const int64_t& ) const noexcept(true);
    void set_compensation(const int &sample_delta,
                           const int &compensation_distance) const noexcept(false);

private:
    SwrContext* m_swr_ctx{};

public:
    ~XSwrContext();
    X_DISABLE_COPY(XSwrContext)
};

SwrContext_sptr new_XSwrContext() noexcept(false);
SwrContext_sptr new_XSwrContext(const AVChannelLayout *out_ch_layout,
                                    const AVSampleFormat &out_sample_fmt,
                                    const int &out_sample_rate,
                                    const AVChannelLayout *in_ch_layout,
                                    const AVSampleFormat &in_sample_fmt,
                                    const int &in_sample_rate,
                                    const int &log_offset = 0,
                                    void *log_ctx  = nullptr) noexcept(false);

#endif
