//
// Created by Administrator on 2024/4/16.
//
#include <iomanip>
#include "Muxer_mp4.h"
#include "AVHelper.h"

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

    init_VideoOutputStream();
    init_AudioOutputStream();
    alloc_yuv_buffer();
    alloc_pcm_buffer();

    m_video_duration = 1.0 / YUV_FPS * TIME_BASE.den;
    m_audio_duration = 1.0 * m_AudioOutputStream->Frame_size() / PCM_SAMPLE_RATE * TIME_BASE.den;
}

void Muxer_mp4::init_AudioOutputStream() noexcept(false)
{
    const Audio_encoder_params audioEncoderParams{AUDIO_BIT_RATE,
                                                  PCM_SAMPLE_RATE};
    const Audio_Resample_Params audioResampleParams{PCM_FMT,CHANNEL_LAYOUT};
    m_AudioOutputStream = new_AudioOutputStream(m_muxer,audioEncoderParams,audioResampleParams);
}

void Muxer_mp4::init_VideoOutputStream() noexcept(false)
{
    const Video_Encoder_params videoEncoderParams{YUV_WIDTH,YUV_HEIGHT};
    m_VideoOutputStream = new_VideoOutputStream(m_muxer,videoEncoderParams);
}

void Muxer_mp4::alloc_yuv_buffer() noexcept(false)
{
    try {
        constexpr auto y_size {YUV_WIDTH * YUV_HEIGHT},u_size{y_size / 4},v_size{y_size / 4};
        m_yuv_buffer_size = y_size + u_size + v_size;

        m_yuv_buffer = static_cast<uint8_t*>(m_mem_pool.allocate(m_yuv_buffer_size));
        std::fill_n(m_yuv_buffer,m_yuv_buffer_size,0);
    } catch (const std::exception &e) {
        throw std::runtime_error("yuv_buffer allocate failed: " + std::string(e.what()) + "\n");
    }
}

void Muxer_mp4::alloc_pcm_buffer() noexcept(false)
{
    try {
        const auto frame_size{m_AudioOutputStream->Frame_size()};

        m_pcm_buffer_size = av_get_bytes_per_sample(PCM_FMT) * CHANNEL_LAYOUT.nb_channels * frame_size;

        m_pcm_buffer = static_cast<uint8_t*>(m_mem_pool.allocate(m_pcm_buffer_size));

        std::fill_n(m_pcm_buffer,m_pcm_buffer_size,0);
    }catch (const std::exception &e){
        throw std::runtime_error("pcm_buffer allocate failed: " + std::string(e.what()) + "\n");
    }
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

void Muxer_mp4::DeConstruct() noexcept(true) {
    m_yuv_file.close();
    m_pcm_file.close();
    m_mem_pool.deallocate(m_yuv_buffer,m_yuv_buffer_size);
    m_mem_pool.deallocate(m_pcm_buffer,m_pcm_buffer_size);
}

Muxer_mp4::~Muxer_mp4() {
    std::cerr << __FUNCTION__  << "\n";
    DeConstruct();
}

Muxer_mp4_sp_type new_Muxer_mp4(const std::string &yuv_file_name,
                                const std::string &pcm_file_name,
                                std::string &&out_file) noexcept(false)
{
    return Muxer_mp4::create(yuv_file_name,pcm_file_name,std::move(out_file));
}

void Muxer_mp4::video_processing() noexcept(false)
{
    if (!video_finish && (audio_finish || m_audio_pts > m_video_pts)){

        std::fill_n(m_yuv_buffer,m_yuv_buffer_size,0);
        m_yuv_file.read(reinterpret_cast<char *>(m_yuv_buffer),m_yuv_buffer_size);

        auto t_yuv_buffer{m_yuv_buffer};
        auto t_yuv_size{m_yuv_file.gcount()};

        if (t_yuv_size <  m_yuv_buffer_size || m_yuv_file.eof()){
            video_finish = true;
            t_yuv_buffer = nullptr;
            t_yuv_size = 0;
        }

        m_VideoOutputStream->encoder(t_yuv_buffer,t_yuv_size,
                                     static_cast<int64_t>(m_video_pts),TIME_BASE,m_packets);
        m_video_pts += m_video_duration;
        for (const auto &i:m_packets) {
            m_muxer->Send_packet(i,TIME_BASE,m_VideoOutputStream->Stream_time_base());
        }

        m_packets.clear();
    }
}

void Muxer_mp4::audio_processing() noexcept(false)
{
    if (!audio_finish){

        std::fill_n(m_pcm_buffer,m_pcm_buffer_size,0);
        m_pcm_file.read(reinterpret_cast<char*>(m_pcm_buffer),m_pcm_buffer_size);
        auto t_pcm_buffer{m_pcm_buffer};
        auto t_pcm_buffer_size{m_pcm_file.gcount()};

        if (t_pcm_buffer_size < m_pcm_buffer_size || m_pcm_file.eof()){
            audio_finish = true;
            t_pcm_buffer = nullptr;
            t_pcm_buffer_size = 0;
        }



    }
}

void Muxer_mp4::exec() noexcept(false)
{
    m_muxer->Send_header();

    while (!video_finish || !audio_finish) {

//        std::cerr << "audio_pts: " <<std::fixed << std::setprecision(5) <<video_pts <<
//                    " video_pts: " << std::fixed << std::setprecision(5) << audio_pts << "\n";

        audio_processing();
        video_processing();
    }

    m_muxer->Send_trailer();
}
