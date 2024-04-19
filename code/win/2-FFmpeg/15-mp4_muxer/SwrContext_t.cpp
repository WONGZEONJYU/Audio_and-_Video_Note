extern "C"{
#include <libavutil/opt.h>
}

#include "SwrContext_t.h"
#include "AVHelper.h"

void SwrContext_t::construct() noexcept(false){
    m_swr_ctx = swr_alloc();
    if (!m_swr_ctx){
        throw std::runtime_error("swr_alloc failed\n");
    }
}

SwrContext_t::SwrContext_sp_t SwrContext_t::create() noexcept(false){

    SwrContext_sp_t obj;

    try {
        obj = std::move(SwrContext_sp_t(new SwrContext_t));
    } catch (const std::bad_alloc &e) {
        throw std::runtime_error("new SwrContext_t failed: " + std::string (e.what()) + "\n");
    }

    try {
        obj->construct();
        return obj;
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw std::runtime_error("SwrContext_t construct failed: " + std::string(e.what()) + "\n");
    }
}

void SwrContext_t::init() const noexcept(false){

    const auto ret {swr_init(m_swr_ctx)};
    if (ret < 0){
        throw std::runtime_error(AVHelper::av_get_err(ret) + "\n");
    }
}

int SwrContext_t::convert(uint8_t **out,const int &out_count,
    const uint8_t **in, const int &in_count) const noexcept(false) {

    auto ret {swr_convert(m_swr_ctx,out,out_count,in,in_count)};
    if (ret < 0){
        throw std::runtime_error("swr_convert failed: " + AVHelper::av_get_err(ret) + "\n");
    }
    return ret;
}

int SwrContext_t::opt_set_ch_layout(const std::string& name, const AVChannelLayout* layout, const int& search_flags) const
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
    std::cerr << __FUNCTION__ << "\n";
    swr_free(&m_swr_ctx);
}
