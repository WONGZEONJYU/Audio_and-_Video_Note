#include <iostream>

extern "C"{
#include <libavutil/opt.h>
#include <libavutil/avutil.h>
#include <libavutil/frame.h>
#include <libavcodec/avcodec.h>
}

#include "Audio_Resampler.h"
#include "AVAudioFifo_t.h"
#include "SwrContext_t.h"
#include "AVHelper.h"

#define FUNCTION_NAME std::string(__FUNCTION__)

constexpr Audio_Resample_Params::Audio_Resample_Params(const AVSampleFormat& src_sample_fmt,
                                                       const AVChannelLayout& src_ch_layout,
                                                       const int &src_sample_rate,
                                                       const AVSampleFormat& dst_sample_fmt,
                                                       const AVChannelLayout& dst_ch_layout,
                                                       const int &dst_sample_rate) noexcept(true) :
                                                       m_src_sample_fmt(src_sample_fmt),
                                                       m_dst_sample_fmt(dst_sample_fmt),
                                                       m_src_ch_layout(src_ch_layout),
                                                       m_dst_ch_layout(dst_ch_layout),
                                                       m_src_sample_rate(src_sample_rate),
                                                       m_dst_sample_rate(dst_sample_rate)
{

}

constexpr Audio_Resample_Params::operator bool() const noexcept(true)
{
    return m_src_sample_fmt == m_dst_sample_fmt &&
            m_src_ch_layout.u.mask == m_dst_ch_layout.u.mask &&
            m_src_sample_rate == m_dst_sample_rate;
}

void Audio_Resample::Construct() noexcept{

    m_audio_fifo = AVAudioFifo_t::create(m_Resample_Params.m_dst_sample_fmt,
                                         m_Resample_Params.m_dst_ch_layout.nb_channels,
                                         1);

    if (m_Resample_Params) {
        m_is_fifo_only.store(true);
        std::cerr << "Audio Resample_Params identical\n";
        return;
    }

    try {
        m_swr_ctx = SwrContext_t::create(
                &m_Resample_Params.m_dst_ch_layout,
                m_Resample_Params.m_dst_sample_fmt,
                m_Resample_Params.m_dst_sample_rate,
                &m_Resample_Params.m_src_ch_layout,
                m_Resample_Params.m_dst_sample_fmt,
                m_Resample_Params.m_dst_sample_rate);
        init_resampled_data();
    } catch (const std::runtime_error &e) {
        m_audio_fifo.reset();
        m_swr_ctx.reset();
        destroy_resample_data();
        throw std::runtime_error(e.what());
    }

    std::cerr << "Audio_Resample init success\n";
}

Audio_Resample_type Audio_Resample::create(const Audio_Resample_Params & params){

    Audio_Resample_type obj;
    try {
        obj = std::move(Audio_Resample_type(new Audio_Resample(params)));
    } catch (const std::bad_alloc &e) {
        throw std::runtime_error("new Audio_Resample failed: " + std::string(e.what()) + "\n");
    }

    try {
        obj->Construct();
        return obj;
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw std::runtime_error("Audio_Resample Construct failed: " + std::string(e.what()) + "\n");
    }
}

Audio_Resample::Audio_Resample(const Audio_Resample_Params &params) noexcept(true):
m_Resample_Params{params}
{
}

void Audio_Resample::init_resampled_data(){
    /*在发送的时候可能遇到空间不足情况,需重新申请,重新申请之前,先释放原来的空间*/
    destroy_resample_data();
    int line_size{};
    const auto dst_channels{m_Resample_Params.m_dst_ch_layout.nb_channels};
    const auto dst_sample_fmt{m_Resample_Params.m_dst_sample_fmt};

    const auto ret {av_samples_alloc_array_and_samples(&m_resampled_data,
                                                        &line_size, dst_channels,
                                                 m_resampled_data_size,
                                                       dst_sample_fmt,
                                                            0)};

    if (ret < 0){
        throw std::runtime_error("alloc audio resampled data buffer failed: " + AVHelper::av_get_err(ret) + "\n");
    }
}

void Audio_Resample::destroy_resample_data(){
    if (m_resampled_data){
        av_freep(&m_resampled_data[0]);
        av_freep(&m_resampled_data);
    }
}

ShareAVFrame_sp_type Audio_Resample::alloc_out_frame(const int &nb_samples) const noexcept(false){

    try {
        auto frame {ShareAVFrame::create()};

        frame->m_frame->nb_samples = nb_samples;
        frame->m_frame->ch_layout = m_Resample_Params.m_dst_ch_layout;
        frame->m_frame->format = m_Resample_Params.m_dst_sample_fmt;
        frame->m_frame->sample_rate = m_Resample_Params.m_dst_sample_rate;

        /*根据提供的信息分配相对应的数据空间*/
        const auto ret{av_frame_get_buffer(frame->m_frame, 0)};
        if (ret < 0) {
            frame.reset();
            throw std::runtime_error(" cannot allocate audio data buffer: " + AVHelper::av_get_err(ret) + "\n");
        }
        return frame;
    } catch (const std::runtime_error &e) {
        throw std::runtime_error(FUNCTION_NAME + "\t" + e.what());
    }
}

int Audio_Resample::fifo_read_helper(uint8_t **out_data, const int &need_nb_samples, int64_t &pts) noexcept(false){

    const auto read_size {m_audio_fifo->read(reinterpret_cast<void**>(out_data),need_nb_samples)};

    pts = m_cur_pts;
    m_cur_pts += need_nb_samples;
    m_total_resampled_num += need_nb_samples;

    return read_size;
}

int Audio_Resample::need_samples_num(const int &nb_samples) const {

    const auto _fifo_size{fifo_size()};

    const auto need_nb_samples{!nb_samples ? _fifo_size : nb_samples};
    /*参数nb_samples为0,则尝试查看fifo有多数数据*/
    if (_fifo_size < need_nb_samples || !need_nb_samples) {
        /*如果fifo为0或fifo数据量小于需要的数据量,则退出当前函数*/
        std::cerr << "Not enough data or fifo is empty\n";
        return {};
    }

    return need_nb_samples;
}

Audio_Resample::~Audio_Resample(){
    destroy_resample_data();
}

int Audio_Resample::send_frame(const AVFrame& frame) {

    const auto src_data{frame.extended_data};     //输入源的buffer
    const auto src_nb_samples{frame.nb_samples};// 输入源采样点数量
    const auto pts{frame.pts};

    return send_frame(src_data,src_nb_samples,pts);
}

int Audio_Resample::send_frame(uint8_t** const in_data, const int& in_nb_samples, const int64_t& pts){

    const auto src_data{in_data};
    const auto src_nb_samples{in_nb_samples};

    if (src_data){
        if (AV_NOPTS_VALUE == m_start_pts && AV_NOPTS_VALUE != pts) {
            m_start_pts = pts;
            m_cur_pts = pts;
        }
    }else{
        m_is_flushed.store(true);
    }

    if (m_is_fifo_only){
        return src_data ? m_audio_fifo->write(reinterpret_cast<void**>(src_data),src_nb_samples) : 0;
    }

    // 计算这次做重采样能够获取到的重采样后的点数
    const auto delay {m_swr_ctx->get_delay(m_Resample_Params.m_src_sample_rate)};

    const auto dst_nb_samples{av_rescale_rnd(delay + src_nb_samples,m_Resample_Params.m_dst_sample_rate,
        m_Resample_Params.m_src_sample_rate,AV_ROUND_UP)};

    try {
        if (dst_nb_samples > m_resampled_data_size) {
            m_resampled_data_size = static_cast<int>(dst_nb_samples);
            init_resampled_data();
        }

        const auto convert_nb_samples{m_swr_ctx->convert(m_resampled_data,static_cast<int>(dst_nb_samples),
                                                         const_cast<const uint8_t**>(src_data),src_nb_samples)};

        return m_audio_fifo->write(reinterpret_cast<void**>(m_resampled_data),convert_nb_samples);

    }catch (const std::runtime_error &e){
        throw std::runtime_error(e.what());
    }
}

int Audio_Resample::send_frame(const uint8_t* in_data, const int& in_bytes, const int64_t& pts) noexcept(false)
{
    AVFrame frame{};
    frame.format = m_Resample_Params.m_src_sample_fmt;
    frame.ch_layout = m_Resample_Params.m_src_ch_layout;

    const auto sample_fmt_size{av_get_bytes_per_sample(m_Resample_Params.m_src_sample_fmt)};
    if (sample_fmt_size < 0){
        throw std::runtime_error("av_get_bytes_per_sample failed: " + AVHelper::av_get_err(sample_fmt_size) + "\n");
    }

    /* 样本数 = 输入的数据数量 ÷ 每个样本的字节数(样本格式大小) ÷ 通道数 */
    frame.nb_samples = in_bytes / sample_fmt_size / frame.ch_layout.nb_channels;
    frame.pts = pts;

    const auto ret{avcodec_fill_audio_frame(&frame, frame.ch_layout.nb_channels,
                                 m_Resample_Params.m_src_sample_fmt, in_data,
                                 in_bytes, 0) };

    if (ret < 0){
        throw std::runtime_error("avcodec_fill_audio_frame failed: " + AVHelper::av_get_err(ret) + "\n");
    }

    return send_frame(frame);
}

ShareAVFrame_sp_type Audio_Resample::receive_frame(const int& nb_samples) noexcept(false){

    const auto need_nb_samples{need_samples_num(nb_samples)};
    if (!need_nb_samples) {
        return {};
    }

    auto frame {alloc_out_frame(need_nb_samples)};

    try {
        fifo_read_helper(frame->m_frame->extended_data,need_nb_samples, frame->m_frame->pts);
        return frame;
    } catch (const std::runtime_error &e) {
        frame.reset();
        throw std::runtime_error(e.what());
    }
}

int Audio_Resample::receive_frame(uint8_t** out_data, const int& nb_samples, int64_t& pts) noexcept(false){

    if (!out_data) {
        throw  std::runtime_error("out_data is empty\n");
    }

    const auto need_nb_samples{need_samples_num(nb_samples)};
    if (!need_nb_samples) {
        return {};
    }

    return fifo_read_helper(out_data, need_nb_samples, pts);
}

int Audio_Resample::fifo_size() const noexcept(true){
    return m_audio_fifo->size();
}
