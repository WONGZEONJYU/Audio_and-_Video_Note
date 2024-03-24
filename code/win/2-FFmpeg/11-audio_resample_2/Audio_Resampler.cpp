#include <iostream>

extern "C"{
#include <libavutil/opt.h>
#include <libavutil/avutil.h>
#include <libavutil/frame.h>
}

#include "Audio_Resampler.h"
#include "AVAudioFifo_t.h"
#include "SwrContext_t.h"


namespace rsmp {

    bool Audio_Resampler::construct() noexcept{

        m_src_channels = m_Resampler_Params.src_ch_layout.u.mask;
        m_dst_channels = m_Resampler_Params.dst_ch_layout.u.mask;

        try{
            m_audio_fifo = AVAudioFifo_t::create(m_Resampler_Params.dst_sample_fmt,
                                                 static_cast<const int >(m_dst_channels),1);
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
        }catch (const std::bad_alloc &e){
            std::cerr << "SwrContext construct failed " << e.what() << "\n";
            return {};
        }
        av_opt_set_in();
        av_opt_set_out();

        return true;
    }

    std::shared_ptr<Audio_Resampler> Audio_Resampler::create(const Audio_Resampler_Params & params)
    {
        try{
           std::shared_ptr<Audio_Resampler> obj{new Audio_Resampler(params)};
            if (!obj->construct()) {
                obj.reset();
                throw std::runtime_error("");
            }
            return obj;
        }catch (const std::exception &e){
            std::cerr << e.what() << "\n";
            throw std::runtime_error("Audio_Resampler construct failed\n");
        }
    }

    Audio_Resampler::Audio_Resampler(const Audio_Resampler_Params & params) {
        m_Resampler_Params = params;
    }

    int Audio_Resampler::init_resampled_data(){
        destory_resampled_data();/*在发送的时候可能遇到空间不足情况,需重新申请,重新申请之前,先释放原来的空间*/
        int linesize{};
        const auto ret {av_samples_alloc_array_and_samples(&m_resampled_data, &linesize, static_cast<int>(m_dst_channels),
                                                     static_cast<int>(m_resampled_data_size), m_Resampler_Params.dst_sample_fmt, 0)};
        if (ret < 0){
            std::cerr << "fail accocate audio resampled data buffer\n";
        }
        return ret;
    }

    void Audio_Resampler::destory_resampled_data(){
        if (m_resampled_data){
            av_freep(&m_resampled_data[0]);
            av_freep(&m_resampled_data);
        }
    }

    void Audio_Resampler::av_opt_set_in() const {
        av_opt_set_chlayout(m_swr_ctx.get(),"in_channel_layout",&m_Resampler_Params.src_ch_layout,0);
        av_opt_set_sample_fmt(m_swr_ctx.get(), "in_sample_fmt", m_Resampler_Params.src_sample_fmt, 0);
        av_opt_set_int(m_swr_ctx.get(),"in_sample_rate",m_Resampler_Params.src_sample_rate,0);

    }

    void Audio_Resampler::av_opt_set_out() const{
        av_opt_set_chlayout(m_swr_ctx.get(),"out_channel_layout",&m_Resampler_Params.dst_ch_layout,0);
        av_opt_set_sample_fmt(m_swr_ctx.get(),"out_sample_fmt",m_Resampler_Params.dst_sample_fmt,0);
        av_opt_set_int(m_swr_ctx.get(),"out_sample_rate",m_Resampler_Params.dst_sample_rate,0);
    }

    Audio_Resampler::~Audio_Resampler(){
        destory_resampled_data();
    }

}
