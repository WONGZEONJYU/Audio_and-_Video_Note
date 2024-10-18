extern "C"{
#include <libswscale/swscale.h>
}
#include "xswscale.hpp"

bool XSwscale::Construct(const int &srcW,const int &srcH,
                         const int &srcFormat,
                         const int &dstW,const int &dstH,
                         const int &dstFormat,
                         const int &flags,
                         SwsFilter *srcFilter,SwsFilter *dstFilter,
                         const double *param) noexcept(true){

    IS_NULLPTR(m_sws_ = sws_getCachedContext(m_sws_,srcW,srcH,
                                               static_cast<AVPixelFormat>(srcFormat),
                                               dstW,dstH,static_cast<AVPixelFormat>(dstFormat),
                                               flags,srcFilter,dstFilter,param),return {});
    return true;
}

void XSwscale::DeConstruct() noexcept(true){
    sws_freeContext(m_sws_);
    m_sws_ = {};
}

int XSwscale::scale(const uint8_t *const *srcSlice,
                       const int *srcStride,
                       const int &srcSliceY,
                       const int &srcSliceH,
                       uint8_t *const *dst,
                       const int *dstStride) const noexcept(true)  {

    if (!m_sws_){
        PRINT_ERR_TIPS(GET_STR(Uninitialized));
        return -1;
    }

    return sws_scale(m_sws_,srcSlice,srcStride,srcSliceY,srcSliceH,dst,dstStride);
}

XSwscale::~XSwscale() {
    DeConstruct();
}

bool XSwscale::reinit(const int &srcW,const int &srcH,const int &srcFormat,
                        const int &dstW,const int &dstH,const int &dstFormat,
                        const int &flags,
                        SwsFilter *srcFilter,SwsFilter *dstFilter,
                        const double * param) noexcept(true) {

    return Construct(srcW,srcH,srcFormat,dstW,dstH,dstFormat,flags,srcFilter,dstFilter,param);
}

XSwscale_sp newXSwscale() {
    XSwscale_sp obj;
    TRY_CATCH(CHECK_EXC(obj = XSwscale_sp(new XSwscale())),return {});
    return obj;
}

XSwscale_sp newXSwscale(const int &srcW,const int &srcH,const int &srcFormat,
                                    const int &dstW,const int &dstH, const int &dstFormat,
                                    const int &flags,
                                    SwsFilter *srcFilter, SwsFilter *dstFilter,
                                    const double *param) noexcept(true) {

    if (auto obj{newXSwscale()}; obj &&
            obj->Construct(srcW,srcH,srcFormat,dstW,dstH,dstFormat,
                                                   flags,srcFilter,dstFilter,param)) {
        return obj;
    }
    return {};
}
