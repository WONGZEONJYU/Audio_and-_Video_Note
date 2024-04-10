
extern "C"{
#include <libswscale/swscale.h>
}

#include "SwsContext_t.h"
#include <iostream>

bool SwsContext_t::construct(const int &srcW,const int &srcH,
                             const AVPixelFormat srcFormat,
                             const int &dstW,const int &dstH,
                             const AVPixelFormat dstFormat,
                             const int &flags,
                             SwsFilter *const srcFilter,
                             SwsFilter *const dstFilter,
                             const double *param) noexcept {

    m_sws = sws_getContext(srcW, srcH, srcFormat,dstW, dstH, dstFormat,
                           flags, srcFilter,dstFilter,param);

    return m_sws;
}

SwsContext_t::SwsContext_t_sp_type SwsContext_t::create(const int &srcW,const int &srcH,
                                                        const AVPixelFormat srcFormat,
                                                        const int &dstW,const int &dstH,
                                                        const AVPixelFormat dstFormat,
                                                        const int &flags,
                                                        SwsFilter *const srcFilter,
                                                        SwsFilter *const dstFilter,
                                                        const double *param) noexcept(false)
{
    try {
        SwsContext_t_sp_type obj(new SwsContext_t);
        if (!obj->construct(srcW,srcH,srcFormat,dstW,dstH,dstFormat,
                            flags,srcFilter,dstFilter,param)){
            obj.reset();
            throw std::runtime_error("sws_getContext failed\n");
        }
        return obj;
    } catch (std::bad_alloc &e) {
        std::cerr << e.what() << "\n";
        throw std::runtime_error("SwsContext_t construct failed\n");
    }
}

SwsContext_t::~SwsContext_t() {
    sws_freeContext(m_sws);
}
