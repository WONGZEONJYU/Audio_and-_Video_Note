//
// Created by Administrator on 2024/7/11.
//

#ifndef PLAYER_SWS_CONTEXT_HPP
#define PLAYER_SWS_CONTEXT_HPP

extern "C"{
#include <libswscale/swscale.h>
}

#include <memory>

class Sws_Context final {
    friend class std::shared_ptr<Sws_Context> newSws_Context(const int &,
                                                             const int &,
                                                             const AVPixelFormat &,
                                                             const int &,
                                                             const int &,
                                                             const AVPixelFormat &,
                                                             const int &,
                                                             SwsFilter *,
                                                             SwsFilter *,
                                                             const double * ) noexcept(false);
    explicit Sws_Context(const int &srcW,
                         const int &srcH,
                         const AVPixelFormat &srcFormat,
                         const int &dstW,
                         const int &dstH,
                         const AVPixelFormat &dstFormat,
                         const int &flags,
                         SwsFilter * srcFilter,
                         SwsFilter * dstFilter,
                         const double * param) noexcept(true);
    void construct() noexcept(false);
    void deconstruct() noexcept(true);
public:
    int scale(const uint8_t * const *srcSlice,
              const int *srcStride,
              const int &srcSliceY,
              const int &srcSliceH,
              uint8_t *const *dst,
              const int *dstStride) noexcept(false);

    void reinit(const int &srcW,
                const int &srcH,
                const AVPixelFormat &srcFormat,
                const int &dstW,
                const int &dstH,
                const AVPixelFormat &dstFormat,
                const int &flags = SWS_BICUBIC,
                SwsFilter * srcFilter = nullptr,
                SwsFilter * dstFilter = nullptr,
                const double *param = nullptr) noexcept(false);
    ~Sws_Context();
private:
    SwsFilter * m_src_filter{},
            * m_dst_filter{};
    SwsContext * m_sws{};

public:
    Sws_Context(const Sws_Context&) = delete;
    Sws_Context& operator=(const Sws_Context&) = delete;
};

using Sws_Context_sptr = std::shared_ptr<Sws_Context>;
Sws_Context_sptr newSws_Context(const int &srcW,
                                const int &srcH,
                                const AVPixelFormat &srcFormat,
                                const int &dstW,
                                const int &dstH,
                                const AVPixelFormat &dstFormat,
                                const int &flags = SWS_BICUBIC,
                                SwsFilter *srcFilter = nullptr,
                                SwsFilter *dstFilter = nullptr,
                                const double *param = nullptr) noexcept(false);
#endif
