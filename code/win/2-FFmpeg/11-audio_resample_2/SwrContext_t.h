
#ifndef SWRCONTEXT_T_H
#define SWRCONTEXT_T_H

extern "C"{
#include <libswresample/swresample.h>
}

#include <memory>

class SwrContext_t final{

    explicit SwrContext_t() = default;
    SwrContext* m_swr_ctx{};
    bool construct();

public:
    using SwrContext_sp_t = std::shared_ptr<SwrContext_t>;
    static SwrContext_sp_t create() noexcept(false);

    SwrContext_t(const SwrContext_t&) = delete;
    SwrContext_t& operator=(const SwrContext_t&) = delete;
    ~SwrContext_t();
    [[nodiscard]] bool init() const;

    int convert	(uint8_t ** out,int out_count,
    const uint8_t ** in,int in_count ) const;
};

#endif //SWRCONTEXT_T_H
