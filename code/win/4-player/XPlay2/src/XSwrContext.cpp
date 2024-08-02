extern "C"{
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}

#include "XSwrContext.hpp"


void XSwrContext::Construct() noexcept(false) {
    CHECK_NULLPTR(m_swr_ctx = swr_alloc());
}

void XSwrContext::Construct(const AVChannelLayout *out_ch_layout,
                            const AVSampleFormat &out_sample_fmt,
                            const int &out_sample_rate,
                            const AVChannelLayout *in_ch_layout,
                            const AVSampleFormat &in_sample_fmt,
                            const int &in_sample_rate,
                            const int &log_offset,
                            void *log_ctx ) noexcept(false) {

    FF_CHECK_ERR(swr_alloc_set_opts2(&m_swr_ctx,
                            out_ch_layout,
                            out_sample_fmt,
                            out_sample_rate,
                            in_ch_layout,
                            in_sample_fmt,
                            in_sample_rate,
                            log_offset,
                            log_ctx));

    init();
}

SwrContext_sptr XSwrContext::create() noexcept(false) {

    SwrContext_sptr obj(new_XSwrContext());

    try {
        obj->Construct();
        return obj;
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw std::runtime_error("XSwrContext construct failed: " + std::string(e.what()) + "\n");
    }
}

SwrContext_sptr XSwrContext::create(const AVChannelLayout *out_ch_layout,
                                       const AVSampleFormat &out_sample_fmt,
                                       const int &out_sample_rate,
                                       const AVChannelLayout *in_ch_layout,
                                       const AVSampleFormat &in_sample_fmt,
                                       const int &in_sample_rate,
                                       const int &log_offset,
                                       void *log_ctx) noexcept(false) {
    SwrContext_sptr obj(new_XSwrContext());

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
        throw std::runtime_error("XSwrContext construct failed: " + std::string(e.what()) + "\n");
    }
}

void XSwrContext::init() const noexcept(false){
    FF_CHECK_ERR(swr_init(m_swr_ctx));
}

int XSwrContext::convert(uint8_t **out, const int &out_count,
                         const uint8_t * const *in, const int &in_count) const noexcept(false) {

    auto ret {-1};
    FF_CHECK_ERR(ret = swr_convert(m_swr_ctx,out,out_count,in,in_count));
    return ret;
}

int XSwrContext::opt_set_ch_layout(const std::string& name,
                                   const AVChannelLayout* layout) const noexcept(true){
   return av_opt_set_chlayout(m_swr_ctx, name.c_str(),layout, 0);
}

int XSwrContext::opt_set_sample_fmt(const std::string& name,
                                    const AVSampleFormat& fmt) const noexcept(true){
    return av_opt_set_sample_fmt(m_swr_ctx, name.c_str(), fmt, 0);
}

int XSwrContext::opt_set_sample_rate(const std::string& name,
                                     const int64_t& val) const noexcept(true){
    return av_opt_set_int(m_swr_ctx, name.c_str(), val, 0);
}

#define FUNCTION_NAME std::string(__FUNCTION__)

void XSwrContext::set_input_ch_layout(const AVChannelLayout *layout) const noexcept(false){
    FF_CHECK_ERR(opt_set_ch_layout("ichl",layout));
}

void XSwrContext::set_input_sample_fmt(const AVSampleFormat &fmt) const noexcept(false){
    FF_CHECK_ERR(opt_set_sample_fmt( "isf", fmt));
}

void XSwrContext::set_input_sample_rate(const int64_t &val) const noexcept(false){
    FF_CHECK_ERR(opt_set_sample_rate("isr", val));
}

void XSwrContext::set_output_ch_layout(const AVChannelLayout *layout) const noexcept(false){
    FF_CHECK_ERR(opt_set_ch_layout("ochl",layout));
}

void XSwrContext::set_output_sample_fmt(const AVSampleFormat &fmt) const noexcept(false){
    FF_CHECK_ERR(opt_set_sample_fmt( "osf", fmt));
}

void XSwrContext::set_output_sample_rate(const int64_t &val) const noexcept(false){
    FF_CHECK_ERR(opt_set_sample_rate("osr", val));
}

int64_t XSwrContext::get_delay(const int64_t& base) const noexcept(true){
    return swr_get_delay(m_swr_ctx,base);
}

void XSwrContext::set_compensation(const int &sample_delta, const int &compensation_distance) const noexcept(false){
    FF_CHECK_ERR(swr_set_compensation(m_swr_ctx,sample_delta,compensation_distance));
}

void XSwrContext::DeConstruct() noexcept(true){
    swr_free(&m_swr_ctx);
}

XSwrContext::~XSwrContext(){
    DeConstruct();
}

XSwrContext* XSwrContext::new_XSwrContext() noexcept(false)
{
    XSwrContext* obj{};
    CHECK_EXC(obj = new XSwrContext);
    return obj;
}

SwrContext_sptr new_XSwrContext() noexcept(false){
    return XSwrContext::create();
}

SwrContext_sptr new_XSwrContext(const AVChannelLayout *out_ch_layout,
                                    const AVSampleFormat &out_sample_fmt,
                                    const int &out_sample_rate,
                                    const AVChannelLayout *in_ch_layout,
                                    const AVSampleFormat &in_sample_fmt,
                                    const int &in_sample_rate,
                                    const int &log_offset,
                                    void *log_ctx) noexcept(false)
{
    return XSwrContext::create(out_ch_layout,
                               out_sample_fmt,
                               out_sample_rate,
                               in_ch_layout,
                               in_sample_fmt,
                               in_sample_rate,
                               log_offset,
                               log_ctx);
}
