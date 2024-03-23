#include <iostream>

extern "C"{
#include <libavutil/fifo.h>
#include <libavutil/opt.h>
#include <libavutil/avutil.h>
#include <libavutil/frame.h>

}

#include "Audio_Resampler.h"

namespace rsmp {

    Audio_Resampler::AVAudioFifo_t::AVAudioFifo_t(const AVSampleFormat &sample_fmt,
                const int& channels,
                const int &nb_samples)

    {
        m_audio_fifo = av_audio_fifo_alloc(sample_fmt,channels,nb_samples);
    }

    Audio_Resampler::AVAudioFifo_t::~AVAudioFifo_t(){
        av_fifo_freep2(reinterpret_cast<AVFifo**>(&m_audio_fifo));
    }

    Audio_Resampler::SwrContext_t::SwrContext_t():
                    m_swr_ctx{swr_alloc()}{}

    Audio_Resampler::SwrContext_t::~SwrContext_t(){
        swr_free(&m_swr_ctx);
    }

    bool Audio_Resampler::construct() noexcept{

        m_src_channels = m_Resampler_Params.src_ch_layout.u.mask;
        m_dst_channels = m_Resampler_Params.dst_ch_layout.u.mask;

        try{
            m_audio_fifo = std::move(std::make_shared<AVAudioFifo_t>
                            (m_Resampler_Params.dst_sample_fmt,
                            m_dst_channels,1));
        }catch (const std::bad_alloc &e){
            std::cerr << "AVAudioFifo construct failed " << e.what() << "\n";
            return {};
        }

        if (m_Resampler_Params) {
            m_is_fifo_only.store(true);
            return true;
        }

        try{
            m_swr_ctx = std::move(std::make_shared<SwrContext_t>());
        }catch (const std::bad_alloc &e){
            std::cerr << "SwrContext construct failed " << e.what() << "\n";
            return {};
        }
        av_opt_set_in();
        av_opt_set_out();

        if (swr_init(m_swr_ctx->m_swr_ctx) < 0){
            std::cerr << "swr_init failed\n";
            m_audio_fifo.reset();
            m_swr_ctx.reset();
            return {};
        }

        return true;
    }

    std::shared_ptr<Audio_Resampler> Audio_Resampler::NewInstance(const Audio_Resampler_Params & params)
    {
        try{
            auto obj{new Audio_Resampler(params)};
            if (obj->construct()){
                return Audio_Resampler_t(obj);
            }else{
                delete obj;
                return {};
            }

        }catch (const std::bad_alloc &e){
            std::cerr << "NewInstance faild : " << e.what() << "\n";
            return {};
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
        av_opt_set_chlayout(m_swr_ctx->m_swr_ctx,"in_channel_layout",&m_Resampler_Params.src_ch_layout,0);
        av_opt_set_sample_fmt(m_swr_ctx->m_swr_ctx, "in_sample_fmt", m_Resampler_Params.src_sample_fmt, 0);
        av_opt_set_int(m_swr_ctx->m_swr_ctx,"in_sample_rate",m_Resampler_Params.src_sample_rate,0);

    }

    void Audio_Resampler::av_opt_set_out() const{
        av_opt_set_chlayout(m_swr_ctx->m_swr_ctx,"out_channel_layout",&m_Resampler_Params.dst_ch_layout,0);
        av_opt_set_sample_fmt(m_swr_ctx->m_swr_ctx,"out_sample_fmt",m_Resampler_Params.dst_sample_fmt,0);
        av_opt_set_int(m_swr_ctx->m_swr_ctx,"out_sample_rate",m_Resampler_Params.dst_sample_rate,0);
    }

    Audio_Resampler::~Audio_Resampler(){
        destory_resampled_data();
    }




}
