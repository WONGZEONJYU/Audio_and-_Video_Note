
#ifndef SWRCONTEXT_T_H
#define SWRCONTEXT_T_H

extern "C"{
#include <libswresample/swresample.h>
}

#include <memory>

class SwrContext_t final{

    explicit SwrContext_t() = default;
    void construct() noexcept(false);

public:
    using SwrContext_sp_t = std::shared_ptr<SwrContext_t>;
    static SwrContext_sp_t create() noexcept(false);

    SwrContext_t(const SwrContext_t&) = delete;
    SwrContext_t& operator=(const SwrContext_t&) = delete;
    ~SwrContext_t();
    void init() const noexcept(false);

    int convert	(uint8_t **out,const int &out_count,
    const uint8_t **in,const int &in_count ) const noexcept(false);

    int opt_set_ch_layout(const std::string& ,const AVChannelLayout *,const int & = 0) const;
    int opt_set_sample_fmt (const std::string&, const AVSampleFormat &,const int & = 0) const;
    int opt_set_rate(const std::string&, const int64_t &val,const int & = 0) const;
    [[nodiscard]]int64_t get_delay(const int64_t& ) const;

private:
    SwrContext* m_swr_ctx{};
};

using SwrContext_sp_t = typename SwrContext_t::SwrContext_sp_t;

#endif
