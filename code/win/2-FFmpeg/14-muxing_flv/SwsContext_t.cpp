extern "C"{
#include <libswscale/swscale.h>
}

#include "SwsContext_t.h"
#include <iostream>
#include "AVHelper.h"

void SwsContext_t::construct(const int &srcW,const int &srcH,
                             const AVPixelFormat &srcFormat,
                             const int &dstW,const int &dstH,
                             const AVPixelFormat &dstFormat,
                             const int &flags,
                             SwsFilter *const srcFilter,
                             SwsFilter *const dstFilter,
                             const double *param) noexcept(false)
{

    m_sws = sws_getContext(srcW, srcH, srcFormat,dstW, dstH, dstFormat,
                           flags, srcFilter,dstFilter,param);
    if (!m_sws){
        throw std::runtime_error("sws_getContext failed\n");
    }
}

SwsContext_t::SwsContext_t_sp_type SwsContext_t::create(const int &srcW,const int &srcH,
                                                        const AVPixelFormat &srcFormat,
                                                        const int &dstW,const int &dstH,
                                                        const AVPixelFormat &dstFormat,
                                                        const int &flags,
                                                        SwsFilter *const srcFilter,
                                                        SwsFilter *const dstFilter,
                                                        const double *param) noexcept(false)
{
    SwsContext_t_sp_type obj;

    try {
        obj = std::move(SwsContext_t_sp_type(new SwsContext_t));
    } catch (const std::bad_alloc &e) {
        throw std::runtime_error("new SwsContext_t failed: " + std::string(e.what()) + "\n");
    }

    try {
        obj->construct(srcW,srcH,srcFormat,dstW,dstH,dstFormat,flags,srcFilter,dstFilter,param);
        return obj;
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw std::runtime_error("SwsContext_t construct failed: " + std::string (e.what()) + "\n");
    }
}

SwsContext_t::~SwsContext_t() {
    std::cerr << __FUNCTION__ << "\n";
    sws_freeContext(m_sws);
}

int SwsContext_t::sws_scale(const uint8_t *const *srcSlice, const int *srcStride, const int &srcSliceY,
                            const int &srcSliceH, uint8_t *const *dst, const int *dstStride) noexcept(false){

    auto ret{::sws_scale(m_sws,srcSlice,srcStride,srcSliceY,srcSliceH,dst,dstStride)};
    if (ret < 0){
        throw std::runtime_error("sws_scale failed: " + AVHelper::av_get_err(ret ) + "\n");
    }

    return ret;
}
