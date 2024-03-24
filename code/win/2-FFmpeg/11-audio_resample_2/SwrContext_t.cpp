#include "SwrContext_t.h"
#include "AVHelper.h"
#include <iostream>

SwrContext_t::SwrContext_sp_t SwrContext_t::create() noexcept(false){
    try {
        SwrContext_sp_t obj(new SwrContext_t);
        if (!obj->construct()) {
            obj.reset();
            throw std::runtime_error("swr_alloc failed\n");
        }
        return obj;
    } catch (std::exception &e) {
        std::cerr << e.what() << "\n";
        throw std::runtime_error("SwrContext_t construct failed\n");
    }
}

bool SwrContext_t::construct() {
    m_swr_ctx = swr_alloc();
    return m_swr_ctx;
}

bool SwrContext_t::init() const{

    const auto ret {swr_init(m_swr_ctx)};

    if( ret < 0 ) {
        std::cerr << AVHelper::av_get_err(ret);
        return false;
    }
    return true;
}

int SwrContext_t::convert(uint8_t **out,const int out_count,
    const uint8_t **in, const int in_count) const{
    return swr_convert(m_swr_ctx,out,out_count,in,in_count);
}

SwrContext_t::~SwrContext_t(){
    swr_free(&m_swr_ctx);
}