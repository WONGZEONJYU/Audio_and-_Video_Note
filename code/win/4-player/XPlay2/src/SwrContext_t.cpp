extern "C"{
#include <libavutil/opt.h>
}

#include "SwrContext_t.hpp"
#include "XHelper.h"

void SwrContext_t::Construct() noexcept(false) {
    m_swr_ctx = swr_alloc();
    if (!m_swr_ctx){
        throw std::runtime_error("swr_alloc failed\n");
    }
}

void SwrContext_t::Construct(const AVChannelLayout *out_ch_layout,
               const AVSampleFormat &out_sample_fmt,
               const int &out_sample_rate,
               const AVChannelLayout *in_ch_layout,
               const AVSampleFormat &in_sample_fmt,
               const int &in_sample_rate,
               const int &log_offset,
               void *log_ctx ) noexcept(false) {
    const auto ret {swr_alloc_set_opts2(&m_swr_ctx,
                            out_ch_layout,
                            out_sample_fmt,
                            out_sample_rate,
                            in_ch_layout,
                            in_sample_fmt,
                            in_sample_rate,
                            log_offset,
                            log_ctx)};
    if (ret < 0){
        throw std::runtime_error("swr_alloc_set_opts2 failed: " + XHelper::av_get_err(ret) + "\n");
    }

    init();
}

SwrContext_sp_type SwrContext_t::create() noexcept(false) {

    SwrContext_sp_type obj(new_SwrContext_t());

    try {
        obj->Construct();
        return obj;
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw std::runtime_error("SwrContext_t construct failed: " + std::string(e.what()) + "\n");
    }
}

SwrContext_sp_type SwrContext_t::create(const AVChannelLayout *out_ch_layout,
                       const AVSampleFormat &out_sample_fmt,
                       const int &out_sample_rate,
                       const AVChannelLayout *in_ch_layout,
                       const AVSampleFormat &in_sample_fmt,
                       const int &in_sample_rate,
                       const int &log_offset,
                       void *log_ctx) noexcept(false) {
    SwrContext_sp_type obj(new_SwrContext_t());

    try {
        obj->Construct(out_ch_layout,
                       out_sample_fmt,
                       out_sample_rate,
                       in_ch_layout,
                       in_sample_fmt,
                       in_sample_rate,
                       log_offset,
                       log_ctx);
        return obj;
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw std::runtime_error("SwrContext_t construct failed: " + std::string(e.what()) + "\n");
    }
}

void SwrContext_t::init() const noexcept(false){

    const auto ret {swr_init(m_swr_ctx)};
    if (ret < 0){
        throw std::runtime_error("swr_init failed: " + XHelper::av_get_err(ret) + "\n");
    }
}

int SwrContext_t::convert(uint8_t **out,const int &out_count,
    const uint8_t * const *in, const int &in_count) const noexcept(false) {

    auto ret {swr_convert(m_swr_ctx,out,out_count,in,in_count)};
    if (ret < 0){
        throw std::runtime_error("swr_convert failed: " + XHelper::av_get_err(ret) + "\n");
    }
    return ret;
}

int SwrContext_t::opt_set_ch_layout(const std::string& name,
                                    const AVChannelLayout* layout) const noexcept(true){
   return av_opt_set_chlayout(m_swr_ctx, name.c_str(),layout, 0);
}

int SwrContext_t::opt_set_sample_fmt(const std::string& name,
                                     const AVSampleFormat& fmt) const noexcept(true){
    return av_opt_set_sample_fmt(m_swr_ctx, name.c_str(), fmt, 0);
}

int SwrContext_t::opt_set_sample_rate(const std::string& name,
                               const int64_t& val) const noexcept(true){
    return av_opt_set_int(m_swr_ctx, name.c_str(), val, 0);
}

#define FUNCTION_NAME std::string(__FUNCTION__)

void SwrContext_t::set_input_ch_layout(const AVChannelLayout *layout) const noexcept(false){
    const auto ret{opt_set_ch_layout("ichl",layout)};
    if (ret < 0){
        throw std::runtime_error( FUNCTION_NAME  + "\t" + XHelper::av_get_err(ret) + "\n");
    }
}

void SwrContext_t::set_input_sample_fmt(const AVSampleFormat &fmt) const noexcept(false){
    const auto ret{opt_set_sample_fmt( "isf", fmt)};
    if (ret < 0){
        throw std::runtime_error( FUNCTION_NAME  + "\t" + XHelper::av_get_err(ret) + "\n");
    }
}

void SwrContext_t::set_input_sample_rate(const int64_t &val) const noexcept(false){
    const auto ret{opt_set_sample_rate("isr", val)};
    if (ret < 0){
        throw std::runtime_error( FUNCTION_NAME  + "\t" + XHelper::av_get_err(ret) + "\n");
    }
}

void SwrContext_t::set_output_ch_layout(const AVChannelLayout *layout) const noexcept(false){
    const auto ret{opt_set_ch_layout("ochl",layout)};
    if (ret < 0){
        throw std::runtime_error( FUNCTION_NAME  + "\t" + XHelper::av_get_err(ret) + "\n");
    }
}

void SwrContext_t::set_output_sample_fmt(const AVSampleFormat &fmt) const noexcept(false){
    const auto ret{opt_set_sample_fmt( "osf", fmt)};
    if (ret < 0){
        throw std::runtime_error( FUNCTION_NAME  + "\t" + XHelper::av_get_err(ret) + "\n");
    }
}

void SwrContext_t::set_output_sample_rate(const int64_t &val) const noexcept(false){
    const auto ret{opt_set_sample_rate("osr", val)};
    if (ret < 0){
        throw std::runtime_error( FUNCTION_NAME  + "\t" + XHelper::av_get_err(ret) + "\n");
    }
}

int64_t SwrContext_t::get_delay(const int64_t& base) const noexcept(true){
    return swr_get_delay(m_swr_ctx,base);
}

void SwrContext_t::set_compensation(const int &sample_delta,const int &compensation_distance) const noexcept(false){
    const auto ret {swr_set_compensation(m_swr_ctx,sample_delta,compensation_distance)};
    if (ret < 0){
        throw std::runtime_error( FUNCTION_NAME  + "\t" + XHelper::av_get_err(ret) + "\n");
    }
}

void SwrContext_t::DeConstruct() noexcept(true){
    swr_free(&m_swr_ctx);
}

SwrContext_t::~SwrContext_t(){
    DeConstruct();
}

SwrContext_t* SwrContext_t::new_SwrContext_t() noexcept(false)
{
    try {
        return new SwrContext_t;
    } catch (const std::bad_alloc &e) {
        throw std::runtime_error("new SwrContext_t failed: " + std::string (e.what()) + "\n");
    }
}

SwrContext_sp_type new_SwrContext_t() noexcept(false){
    return SwrContext_t::create();
}

SwrContext_sp_type new_SwrContext_t(const AVChannelLayout *out_ch_layout,
                                    const AVSampleFormat &out_sample_fmt,
                                    const int &out_sample_rate,
                                    const AVChannelLayout *in_ch_layout,
                                    const AVSampleFormat &in_sample_fmt,
                                    const int &in_sample_rate,
                                    const int &log_offset,
                                    void *log_ctx) noexcept(false)
{
    return SwrContext_t::create(out_ch_layout,
                                out_sample_fmt,
                                out_sample_rate,
                                in_ch_layout,
                                in_sample_fmt,
                                in_sample_rate,
                                log_offset,
                                log_ctx);
}
