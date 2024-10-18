extern "C"{
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}

#include "xswrsample.hpp"

bool XSwrSample::Construct() noexcept(true) {
    IS_NULLPTR(m_swr_ctx_ = swr_alloc(),return {});
    return true;
}

bool XSwrSample::Construct(const AVChannelLayout *out_ch_layout,
                            const int &out_sample_fmt,
                            const int &out_sample_rate,
                            const AVChannelLayout *in_ch_layout,
                            const int &in_sample_fmt,
                            const int &in_sample_rate,
                            const int &log_offset,
                            void *log_ctx) noexcept(true) {

    FF_ERR_OUT(swr_alloc_set_opts2(&m_swr_ctx_,
                            out_ch_layout,
                            static_cast<AVSampleFormat>(out_sample_fmt),
                            out_sample_rate,
                            in_ch_layout,
                            static_cast<AVSampleFormat>(in_sample_fmt),
                            in_sample_rate,
                            log_offset,
                            log_ctx),return {});

    return init();
}

bool XSwrSample::init() const noexcept(true){
    FF_ERR_OUT(swr_init(m_swr_ctx_),return {});
    return true;
}

int XSwrSample::convert(uint8_t **out, const int &out_count,
                         const uint8_t * const *in, const int &in_count) const noexcept(false) {

    int ret;
    FF_CHECK_ERR(ret = swr_convert(m_swr_ctx_,out,out_count,in,in_count));
    return ret;
}

int XSwrSample::opt_set_ch_layout(const std::string& name,
                                   const AVChannelLayout* layout) const noexcept(true){
   return av_opt_set_chlayout(m_swr_ctx_,name.c_str(),layout, 0);
}

int XSwrSample::opt_set_sample_fmt(const std::string& name,
                                    const AVSampleFormat& fmt) const noexcept(true){
    return av_opt_set_sample_fmt(m_swr_ctx_,name.c_str(), fmt, 0);
}

int XSwrSample::opt_set_sample_rate(const std::string& name,
                                     const int64_t& val) const noexcept(true){
    return av_opt_set_int(m_swr_ctx_,name.c_str(), val, 0);
}

bool XSwrSample::set_input_ch_layout(const AVChannelLayout *layout) const noexcept(true){
    FF_ERR_OUT(opt_set_ch_layout(GET_STR(ichl),layout),return {});
    return true;
}

bool XSwrSample::set_input_sample_fmt(const int &fmt) const noexcept(true){
    FF_ERR_OUT(opt_set_sample_fmt(GET_STR(isf), static_cast<AVSampleFormat>(fmt)),return {});
    return true;
}

bool XSwrSample::set_input_sample_rate(const int64_t &val) const noexcept(true){
    FF_ERR_OUT(opt_set_sample_rate(GET_STR(isr), val),return {});
    return true;
}

bool XSwrSample::set_output_ch_layout(const AVChannelLayout *layout) const noexcept(true){
    FF_ERR_OUT(opt_set_ch_layout(GET_STR(ochl),layout),return {});
    return true;
}

bool XSwrSample::set_output_sample_fmt(const int &fmt) const noexcept(true){
    FF_ERR_OUT(opt_set_sample_fmt( GET_STR(osf), static_cast<AVSampleFormat>(fmt)),return {});
    return true;
}

bool XSwrSample::set_output_sample_rate(const int64_t &val) const noexcept(true){
    FF_ERR_OUT(opt_set_sample_rate(GET_STR(osr), val),return {});
    return true;
}

int64_t XSwrSample::get_delay(const int64_t& base) const noexcept(true){
    return swr_get_delay(m_swr_ctx_,base);
}

bool XSwrSample::set_compensation(const int &sample_delta,
    const int &compensation_distance) const noexcept(true){
    FF_ERR_OUT(swr_set_compensation(m_swr_ctx_,sample_delta,compensation_distance),return {});
    return true;
}

void XSwrSample::DeConstruct() noexcept(true){
    swr_free(&m_swr_ctx_);
}

XSwrSample::~XSwrSample(){
    DeConstruct();
}

auto XSwrSample::new_obj()->XSwrSample_sp {
    XSwrSample_sp obj;
    TRY_CATCH(CHECK_EXC(obj.reset(new XSwrSample())),return {});
    return obj;
}

XSwrSample_sp new_XSwrSample() noexcept(true) {
    if (auto obj{XSwrSample::new_obj()};obj &&
        obj->Construct()) {
        return obj;
    }
    return {};
}

XSwrSample_sp new_XSwrSample(const AVChannelLayout *out_ch_layout,
                                    const int &out_sample_fmt,
                                    const int &out_sample_rate,
                                    const AVChannelLayout *in_ch_layout,
                                    const int &in_sample_fmt,
                                    const int &in_sample_rate,
                                    const int &log_offset,
                                    void *log_ctx) noexcept(true)
{
    if (auto obj{XSwrSample::new_obj()};
        obj && obj->Construct(out_ch_layout,out_sample_fmt,out_sample_rate,
        in_ch_layout,in_sample_fmt,in_sample_rate,log_offset,log_ctx)) {
        return obj;
    }

    return {};
}
