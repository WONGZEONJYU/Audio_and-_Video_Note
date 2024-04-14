
#ifndef SWRCONTEXT_T_H
#define SWRCONTEXT_T_H

extern "C"{
#include <libswresample/swresample.h>
}

#include <memory>

class SwrContext_t final{

    SwrContext* m_swr_ctx{};
    explicit SwrContext_t() = default;
    bool construct();

public:
    using SwrContext_sp_t = std::shared_ptr<SwrContext_t>;
    static SwrContext_sp_t create() noexcept(false);

    SwrContext_t(const SwrContext_t&) = delete;
    SwrContext_t& operator=(const SwrContext_t&) = delete;
    ~SwrContext_t();
    [[nodiscard]] bool init() const;

    int convert	(uint8_t **out,const int &out_count,
    const uint8_t **in,const int &in_count ) const;

    int opt_set_chlayout(const std::string& ,const AVChannelLayout *,const int &search_flags = 0) const;
    int opt_set_sample_fmt (const std::string&, const AVSampleFormat &,const int &search_flags = 0) const;
    int opt_set_rate( const std::string&, const int64_t &val,const int &search_flags = 0) const;
    int64_t get_delay(const int64_t& base) const;
};

#endif //SWRCONTEXT_T_H
