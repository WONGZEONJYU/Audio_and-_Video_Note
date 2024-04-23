//
// Created by Administrator on 2024/4/16.
//

#include "Muxer_mp4.h"
#include "VideoEncoder.h"
#include "AudioEncoder.h"

Muxer_mp4::Muxer_mp4(const std::string &yuv_file_name,const std::string &pcm_file_name)noexcept(true):
            m_yuv_file(yuv_file_name,std::ios::binary),
            m_pcm_file(pcm_file_name,std::ios::binary),
            resource(PACKETS_SIZE),
            m_packets(&resource)
{
}

void Muxer_mp4::Construct(std::string &&url) noexcept(false)
{
    if (!m_yuv_file){
        throw std::runtime_error("open m_yuv_file failed\n");
    }

    if (!m_pcm_file){
        throw std::runtime_error("open m_yuv_file failed\n");
    }

    m_muxer = new_Muxer(std::move(url));

    const Audio_encoder_params audioEncoderParams{AUDIO_BIT_RATE,
                           PCM_SAMPLE_RATE,};

    const Audio_Resample_Params audioResampleParams{PCM_FMT,CHANNEL_LAYOUT};

    m_AudioOutputStream = new_AudioOutputStream(m_muxer,audioEncoderParams,audioResampleParams);

    const Video_Encoder_params videoEncoderParams{YUV_WIDTH,YUV_HEIGHT};

    m_VideoOutputStream = new_VideoOutputStream(m_muxer,videoEncoderParams);


}

Muxer_mp4_sp_type Muxer_mp4::create(const std::string &yuv_file_name,
                                    const std::string &pcm_file_name,
                                    std::string&& out_file) noexcept(false) {

    Muxer_mp4_sp_type obj;
    try {
        obj = std::move(Muxer_mp4_sp_type(new Muxer_mp4(yuv_file_name,pcm_file_name)));
    } catch (const std::bad_alloc &e) {
        throw std::runtime_error("new Muxer_mp4 failed\n");
    }

    try {
        obj->Construct(std::move(out_file));
        return obj;
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw std::runtime_error("");
    }
}

Muxer_mp4::~Muxer_mp4() {
    DeConstruct();
}

void Muxer_mp4::DeConstruct() noexcept(true) {
    m_yuv_file.close();
    m_pcm_file.close();
    m_mem_pool.deallocate(m_yuv_buffer,0);
    m_mem_pool.deallocate(m_pcm_buffer,0);
}

Muxer_mp4_sp_type new_Muxer_mp4(const std::string &yuv_file_name,
                                const std::string &pcm_file_name,
                                std::string &&out_file) noexcept(false)
{
    return Muxer_mp4::create(yuv_file_name,pcm_file_name,std::move(out_file));
}
