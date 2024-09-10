//
// Created by Administrator on 2024/7/11.
//

#include "xswscontext.hpp"

XSwsContext::XSwsContext(const int &srcW,const int &srcH,
                         const AVPixelFormat &srcFormat,
                         const int &dstW,const int &dstH,
                         const AVPixelFormat &dstFormat,
                         const int &flags,
                         SwsFilter *srcFilter,SwsFilter *dstFilter,
                         const double *param) noexcept(true):
                         m_src_filter{srcFilter},
                         m_dst_filter{dstFilter},
                         m_sws{sws_getCachedContext({},srcW,srcH,srcFormat,
                             dstW,dstH,dstFormat,flags,
                             srcFilter,dstFilter,param)}{
}

void XSwsContext::Construct() noexcept(false){
    CHECK_NULLPTR(m_sws);
}

void XSwsContext::DeConstruct() noexcept(true){
    sws_freeContext(m_sws);
    m_sws = {};
}

int XSwsContext::scale(const uint8_t *const *srcSlice,
                       const int *srcStride,
                       const int &srcSliceY,
                       const int &srcSliceH,
                       uint8_t *const *dst,
                       const int *dstStride) noexcept(false) {

    if (!m_sws){
        PRINT_ERR_TIPS(GET_STR(Uninitialized));
        return -1;
    }

    return sws_scale(m_sws,srcSlice,srcStride,srcSliceY,srcSliceH,dst,dstStride);
}

XSwsContext::~XSwsContext() {
    DeConstruct();
}

void XSwsContext::reinit(const int &srcW,const int &srcH,
                         const AVPixelFormat &srcFormat,
                         const int &dstW,const int &dstH,
                         const AVPixelFormat &dstFormat,
                         const int &flags,
                         SwsFilter *srcFilter,SwsFilter *dstFilter,
                         const double * param) noexcept(false) {
    CHECK_NULLPTR(m_sws = sws_getCachedContext(m_sws,srcW,srcH,srcFormat,dstW,dstH,dstFormat,
                                               flags,srcFilter,dstFilter,param));
}

XSwsContext_sptr newXSwsContext(){
    XSwsContext_sptr obj;
    CHECK_EXC(obj.reset(new XSwsContext()));
    return obj;
}

XSwsContext_sptr newXSwsContext(const int &srcW,const int &srcH,
                                const AVPixelFormat &srcFormat,
                                const int &dstW,const int &dstH,
                                const AVPixelFormat &dstFormat,
                                const int &flags,
                                SwsFilter *srcFilter,SwsFilter *dstFilter,
                                const double *param)  noexcept(false){
    XSwsContext_sptr obj;
    CHECK_EXC(obj.reset(new XSwsContext(srcW,srcH,srcFormat,dstW,dstH,dstFormat,
                                        flags,srcFilter,dstFilter,param)));
    CHECK_EXC(obj->Construct(),obj.reset());
    return obj;
}
