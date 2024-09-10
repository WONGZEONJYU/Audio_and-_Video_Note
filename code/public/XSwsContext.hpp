//
// Created by Administrator on 2024/7/11.
//

#ifndef PLAYER_SWS_CONTEXT_HPP
#define PLAYER_SWS_CONTEXT_HPP

extern "C"{
#include <libswscale/swscale.h>
}

#include <memory>
#include "xhelper.hpp"

class XSwsContext;
using XSwsContext_sptr = std::shared_ptr<XSwsContext>;

class XSwsContext final {

    friend XSwsContext_sptr newXSwsContext();
    friend XSwsContext_sptr newXSwsContext(const int &,const int &,
                                        const AVPixelFormat &,
                                        const int &,const int &,
                                        const AVPixelFormat &,
                                        const int &,
                                        SwsFilter *,SwsFilter *,
                                        const double *) noexcept(false);

    explicit XSwsContext() = default;
    explicit XSwsContext(const int &srcW,const int &srcH,
                         const AVPixelFormat &srcFormat,
                         const int &dstW,const int &dstH,
                         const AVPixelFormat &dstFormat,
                         const int &flags,
                         SwsFilter *srcFilter,SwsFilter *dstFilter,
                         const double *param) noexcept(true);

    void Construct() noexcept(false);
    void DeConstruct() noexcept(true);
public:
    int scale(const uint8_t *const *srcSlice,
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
                const int &flags = SWS_BILINEAR,
                SwsFilter *srcFilter = {},
                SwsFilter *dstFilter = {},
                const double *param = {}) noexcept(false);
    ~XSwsContext() ;
private:
    SwsFilter *m_src_filter{},
            *m_dst_filter{};
    SwsContext *m_sws{};

public:
    X_DISABLE_COPY_MOVE(XSwsContext)
};

XSwsContext_sptr newXSwsContext();
XSwsContext_sptr newXSwsContext(const int &srcW,
                                const int &srcH,
                                const AVPixelFormat &srcFormat,
                                const int &dstW,
                                const int &dstH,
                                const AVPixelFormat &dstFormat,
                                const int &flags = SWS_BILINEAR,
                                SwsFilter *srcFilter = nullptr,
                                SwsFilter *dstFilter = nullptr,
                                const double *param = nullptr) noexcept(false);
#endif
