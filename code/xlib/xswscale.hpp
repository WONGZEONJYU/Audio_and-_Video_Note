#ifndef XSWSCALE_HPP_
#define XSWSCALE_HPP_

#include "xhelper.hpp"

class XLIB_API XSwscale final {

    explicit XSwscale() = default;

    [[nodiscard]] bool Construct(const int &srcW,
                                const int &srcH,
                                const int &srcFormat,
                                const int &dstW,
                                const int &dstH,
                                const int &dstFormat,
                                const int &flags,
                                SwsFilter *srcFilter,SwsFilter *dstFilter,
                                const double * param) noexcept(true);

    void DeConstruct() noexcept(true);
public:
    int scale(const uint8_t *const *srcSlice,
              const int *srcStride,
              const int &srcSliceY,
              const int &srcSliceH,
              uint8_t *const *dst,
              const int *dstStride) const noexcept(true) ;

    [[nodiscard]] bool reinit(const int &srcW,
                const int &srcH,
                const int &srcFormat,
                const int &dstW,
                const int &dstH,
                const int &dstFormat,
                const int &flags = SWS_BILINEAR,
                SwsFilter *srcFilter = {},
                SwsFilter *dstFilter = {},
                const double *param = {}) noexcept(true);
private:
    SwsContext *m_sws_{};

public:
    friend XLIB_API XSwscale_sp newXSwscale();
    friend XLIB_API XSwscale_sp newXSwscale(const int &srcW,
                                const int &srcH,
                                const int &srcFormat,
                                const int &dstW,
                                const int &dstH,
                                const int &dstFormat,
                                const int &flags ,
                                SwsFilter *srcFilter ,
                                SwsFilter *dstFilter ,
                                const double *param ) noexcept(true);
    ~XSwscale() ;
    X_DISABLE_COPY_MOVE(XSwscale)
};

XLIB_API XSwscale_sp newXSwscale();
XLIB_API XSwscale_sp newXSwscale(const int &srcW,
                            const int &srcH,
                            const int &srcFormat,
                            const int &dstW,
                            const int &dstH,
                            const int &dstFormat,
                            const int &flags = SWS_BILINEAR,
                            SwsFilter *srcFilter = {},
                            SwsFilter *dstFilter = {},
                            const double *param = {}) noexcept(true);

#endif
