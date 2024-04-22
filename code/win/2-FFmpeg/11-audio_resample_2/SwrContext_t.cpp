extern "C"{
#include <libavutil/opt.h>
}

#include "SwrContext_t.h"
#include "AVHelper.h"
#include <iostream>

bool SwrContext_t::construct() {
    m_swr_ctx = swr_alloc();
    return m_swr_ctx;
}

SwrContext_t::SwrContext_sp_t SwrContext_t::create() noexcept(false){

    try {
        SwrContext_sp_t obj(new SwrContext_t);
        if (!obj->construct()) {
            obj.reset();
            throw std::runtime_error("swr_alloc failed\n");
        }
        return obj;
    } catch (std::bad_alloc &e) {
        std::cerr << e.what() << "\n";
        throw std::runtime_error("SwrContext_t construct failed\n");
    }
}

bool SwrContext_t::init() const{

    const auto ret {swr_init(m_swr_ctx)};
    return ret >= 0 || (std::cerr << AVHelper::av_get_err(ret), false);
}

int SwrContext_t::convert(uint8_t **out,const int &out_count,
    const uint8_t **in, const int &in_count) const {
    return swr_convert(m_swr_ctx,out,out_count,in,in_count);
}

int SwrContext_t::opt_set_chlayout(const std::string& name, const AVChannelLayout* layout, const int& search_flags) const
{
    return av_opt_set_chlayout(m_swr_ctx, name.c_str(),layout, search_flags);
}

int SwrContext_t::opt_set_sample_fmt(const std::string& name, const AVSampleFormat& fmt, const int& search_flags) const
{
    return av_opt_set_sample_fmt(m_swr_ctx,name.c_str(),fmt, search_flags);
}

int SwrContext_t::opt_set_rate(const std::string& name, const int64_t& val, const int& search_flags) const
{
    return av_opt_set_int(m_swr_ctx,name.c_str(), val, search_flags);
}

int64_t SwrContext_t::get_delay(const int64_t& base) const
{
    return swr_get_delay(m_swr_ctx,base);
}

SwrContext_t::~SwrContext_t(){
    swr_free(&m_swr_ctx);
}
