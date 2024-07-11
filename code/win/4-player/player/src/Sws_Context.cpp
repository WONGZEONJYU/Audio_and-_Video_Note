//
// Created by Administrator on 2024/7/11.
//

#include "Sws_Context.hpp"

Sws_Context::Sws_Context(const int &srcW,
                         const int &srcH,
                         const AVPixelFormat &srcFormat,
                         const int &dstW,
                         const int &dstH,
                         const AVPixelFormat &dstFormat,
                         const int &flags,
                         SwsFilter * srcFilter,
                         SwsFilter * dstFilter,
                         const double * param) noexcept(true):
                         m_src_filter{srcFilter},
                         m_dst_filter{dstFilter},
                         m_sws{sws_getContext(srcW,srcH,srcFormat,
                                                    dstW,dstH,dstFormat,flags,
                                                    srcFilter,dstFilter,param)} {

}

void Sws_Context::construct() noexcept(false){
    if (!m_sws){
        throw std::runtime_error("SwsContext alloc failed");
    }
}

void Sws_Context::deconstruct() noexcept(true){
    sws_freeContext(m_sws);
    m_sws = nullptr;
}

int Sws_Context::scale(const uint8_t *const *srcSlice,
                       const int *srcStride,
                       const int &srcSliceY,
                       const int &srcSliceH,
                       uint8_t *const *dst,
                       const int *dstStride) noexcept(false) {
    if (!m_sws) {
        throw std::runtime_error("SwsContext is null");
    }
    return sws_scale(m_sws,srcSlice,srcStride,srcSliceY,srcSliceH,dst,dstStride);
}

Sws_Context::~Sws_Context() {
    deconstruct();
}

void Sws_Context::reinit(const int &srcW,
                         const int &srcH,
                         const AVPixelFormat &srcFormat,
                         const int &dstW,
                         const int &dstH,
                         const AVPixelFormat &dstFormat,
                         const int &flags,
                         SwsFilter *srcFilter,
                         SwsFilter *dstFilter,
                         const double * param) noexcept(false){

    m_sws = sws_getCachedContext(m_sws,srcW,srcH,srcFormat,dstW,dstH,dstFormat,flags,srcFilter,dstFilter,param);
    if (!m_sws){
        throw std::runtime_error("SwsContext reinit failed");
    }
}

Sws_Context_sptr newSws_Context(const int &srcW,
                                const int &srcH,
                                const AVPixelFormat &srcFormat,
                                const int &dstW,
                                const int &dstH,
                                const AVPixelFormat &dstFormat,
                                const int &flags,
                                SwsFilter *srcFilter,
                                SwsFilter *dstFilter,
                                const double *param)  noexcept(false){
    Sws_Context_sptr obj;
    try {
        obj.reset(new Sws_Context(srcW,srcH,srcFormat,dstW,dstH,dstFormat,flags,srcFilter,dstFilter,param));
        obj->construct();
        return obj;
    } catch (const std::bad_alloc &e) {
        obj.reset();
        throw std::runtime_error("new Sws_Context failed");
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw e;
    }
}
