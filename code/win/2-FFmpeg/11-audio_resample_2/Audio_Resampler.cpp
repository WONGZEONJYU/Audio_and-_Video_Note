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

namespace rsmp {

    bool Audio_Resampler::construct() noexcept{

        m_src_channels = m_Resampler_Params.src_ch_layout.nb_channels;
        m_dst_channels = m_Resampler_Params.dst_ch_layout.nb_channels;

        try{
            m_audio_fifo = AVAudioFifo_t::create(m_Resampler_Params.dst_sample_fmt,
                                                 m_dst_channels,1);
        }catch (const std::exception &e){
            std::cerr << "AVAudioFifo construct failed " << e.what() << "\n";
            return {};
        }

        if (m_Resampler_Params) {
            m_is_fifo_only.store(true);
            return true;
        }

        try{
            m_swr_ctx = SwrContext_t::create();
        }catch (const std::exception &e){
            std::cerr << "SwrContext construct failed " << e.what() << "\n";
            return {};
        }

        av_opt_set_in();
        av_opt_set_out();

        if (!m_swr_ctx->init()){
            std::cerr << "failed to initialize the resampling context.\n";
            m_audio_fifo.reset();
            m_swr_ctx.reset();
            destory_resampled_data();
            return {};
        }

        if (!init_resampled_data()){
            std::cerr << "alloc resampled_data failed\n";
            m_audio_fifo.reset();
            m_swr_ctx.reset();
            destory_resampled_data();
            return {};
        }

        std::cout << "construct done\n" << std::flush;

        return true;
    }

    std::shared_ptr<Audio_Resampler> Audio_Resampler::create(const Audio_Resampler_Params & params){

        try{
           std::shared_ptr<Audio_Resampler> obj{new Audio_Resampler(params)};
            if (!obj->construct()) {
                obj.reset();
                throw std::runtime_error("Audio_Resampler init failed\n");
            }
            return obj;
        }catch (const std::bad_alloc &e){
            std::cerr << e.what() << "\n";
            throw std::runtime_error("Audio_Resampler construct failed\n");
        }
    }

    Audio_Resampler::Audio_Resampler(const Audio_Resampler_Params &params):
    m_Resampler_Params{params} {
    }

    bool Audio_Resampler::init_resampled_data(){
        /*在发送的时候可能遇到空间不足情况,需重新申请,重新申请之前,先释放原来的空间*/
        destory_resampled_data();
        int linesize{};
        const auto ret {av_samples_alloc_array_and_samples(&m_resampled_data,
                                                            &linesize, m_dst_channels,
                                                     static_cast<int>(m_resampled_data_size),
                                                                m_Resampler_Params.dst_sample_fmt,
                                                                0)};

        return ret >= 0 || (std::cerr << "fail accocate audio resampled data buffer errcode : " <<
                                      ret << "\t" << AVHelper::av_get_err(ret) << "\n", false);
    }

    void Audio_Resampler::destory_resampled_data(){
        if (m_resampled_data){
            av_freep(&m_resampled_data[0]);
            av_freep(&m_resampled_data);
        }
    }

    void Audio_Resampler::av_opt_set_in() const {
        auto ret = m_swr_ctx->opt_set_chlayout("ichl",&m_Resampler_Params.src_ch_layout);
        std::cerr << AVHelper::av_get_err(ret) << "\n";
        ret = m_swr_ctx->opt_set_sample_fmt("isf",m_Resampler_Params.src_sample_fmt);
        ret = m_swr_ctx->opt_set_rate("isr",m_Resampler_Params.src_sample_rate);
    }

    void Audio_Resampler::av_opt_set_out() const{
        auto ret = m_swr_ctx->opt_set_chlayout("ochl",&m_Resampler_Params.dst_ch_layout);
        ret = m_swr_ctx->opt_set_sample_fmt("osf",m_Resampler_Params.dst_sample_fmt);
        ret = m_swr_ctx->opt_set_rate("osr",m_Resampler_Params.dst_sample_rate);
    }

    AVFrame * Audio_Resampler::alloc_out_frame(const int &nb_samples) const{

        auto frame {av_frame_alloc()};

        if (frame) {
            frame->nb_samples = nb_samples;
            frame->ch_layout = m_Resampler_Params.dst_ch_layout;
            frame->format = m_Resampler_Params.dst_sample_fmt;
            frame->sample_rate = m_Resampler_Params.dst_sample_rate;
            const auto ret{av_frame_get_buffer(frame, 0)};
            /*根据提供的信息分配相对应的数据空间*/
            if (ret < 0) {
                av_frame_free(&frame);
                std::cerr << "cannot allocate audio data buffer : " << ret << "\t" <<
                AVHelper::av_get_err(ret) << "\n";
            }
        }

        return frame;
    }

    int Audio_Resampler::fifo_read_helper(uint8_t **out_data, const int &need_nb_samples, int64_t &pts) {

        const auto read_size {m_audio_fifo->read(reinterpret_cast<void**>(out_data),need_nb_samples)};

        if (read_size < 0) {
            std::cerr << "avaudio_fifo_read failed errcode : " << read_size << "\t" <<
                AVHelper::av_get_err(read_size) << "\n";
            return read_size;
        }

        pts = m_cur_pts;
        m_cur_pts += need_nb_samples;
        m_total_resampled_num += need_nb_samples;

        return read_size;
    }

    int Audio_Resampler::need_samples_num(const int &nb_samples) const {

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

    Audio_Resampler::~Audio_Resampler(){
        destory_resampled_data();
    }

    int Audio_Resampler::send_frame(const AVFrame& frame) {

        const auto src_data{frame.extended_data};     //输入源的buffer
        const auto src_nb_samples{frame.nb_samples};// 输入源采样点数量
        const auto pts{frame.pts};

        return send_frame(src_data,src_nb_samples,pts);
    }

    int Audio_Resampler::send_frame(uint8_t** const in_data, const int& in_nb_samples, const int64_t& pts){

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
        const auto delay {m_swr_ctx->get_delay(m_Resampler_Params.src_sample_rate)};

        const auto dst_nb_samples{av_rescale_rnd(delay + src_nb_samples,m_Resampler_Params.dst_sample_rate,
            m_Resampler_Params.src_sample_rate,AV_ROUND_UP)};

        if (dst_nb_samples > m_resampled_data_size){
            m_resampled_data_size = dst_nb_samples;
            if (!init_resampled_data()){
                return AVERROR(ENOMEM);
            }
        }

        const auto convert_nb_samples{m_swr_ctx->convert(m_resampled_data,static_cast<int>(dst_nb_samples),
                                                         const_cast<const uint8_t**>(src_data),src_nb_samples)};

        const auto w_nb{m_audio_fifo->write(reinterpret_cast<void**>(m_resampled_data),convert_nb_samples)};

        if (w_nb < 0){
            std::cerr << "av_audio_fifo_write failed errcode : "<< w_nb << "\t" << AVHelper::av_get_err(w_nb) << "\n";
        }

        return w_nb;
    }

    int Audio_Resampler::send_frame(const uint8_t* in_data, const int& in_bytes, const int64_t& pts){

        AVFrame frame{};
        frame.format = m_Resampler_Params.src_sample_fmt;
        frame.ch_layout = m_Resampler_Params.src_ch_layout;
        const auto sample_fmt_size{av_get_bytes_per_sample(m_Resampler_Params.src_sample_fmt)};
        /*输入的数据数量 ÷ 每个样本的字节数(样本格式大小) ÷ 通道数 */
        frame.nb_samples = in_bytes / sample_fmt_size / frame.ch_layout.nb_channels;
        frame.pts = pts;

        avcodec_fill_audio_frame(&frame, frame.ch_layout.nb_channels,
                             m_Resampler_Params.src_sample_fmt, in_data,
                             in_bytes, 0);

        return send_frame(frame);
    }

    AVFrame* Audio_Resampler::receive_frame(const int& nb_samples){

        const auto need_nb_samples{need_samples_num(nb_samples)};
        if (!need_nb_samples) {
            return {};
        }

        auto frame {alloc_out_frame(need_nb_samples)};

        if (frame) {
            if (fifo_read_helper(frame->extended_data,need_nb_samples, frame->pts) < 0) {
                av_frame_free(&frame);
            }
        }

        return frame;
    }

    int Audio_Resampler::receive_frame(uint8_t** out_data, const int& nb_samples, int64_t& pts){

        if (!out_data) {
            std::cerr << "out_data is empty\n";
            return AVERROR(ENOMEM);
        }

        const auto need_nb_samples{need_samples_num(nb_samples)};
        if (!need_nb_samples) {
            return {};
        }

        return fifo_read_helper(out_data, need_nb_samples, pts);
    }

    int Audio_Resampler::fifo_size() const{
        return m_audio_fifo->size();
    }

    int64_t Audio_Resampler::start_pts() const{
        return m_start_pts;
    }

    int64_t Audio_Resampler::cur_pts() const{
        return m_cur_pts;
    }
}
