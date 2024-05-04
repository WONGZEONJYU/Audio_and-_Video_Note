//
// Created by wong on 2024/5/4.
//

#include "Mixer.hpp"
Mixer::Mixer(const char **argv) noexcept(true) :
m_input_file_1(argv[1],std::ios::binary),
m_input_file_2(argv[2],std::ios::binary),
m_output_file(argv[3],std::ios::binary),
m_pcm_flt_buf_size(av_get_bytes_per_sample(FMT_0) * CH_0.nb_channels * 1024),
m_pcm_s16_buf_size(av_get_bytes_per_sample(FMT_1) * CH_1.nb_channels * 1024),
m_out_buf_size(m_pcm_flt_buf_size + m_pcm_s16_buf_size)
{
}

void Mixer::Construct() noexcept(false)
{
    if (!m_input_file_1){
        throw std::runtime_error("open file_1 failed\n");
    }

    if (!m_input_file_2){
        throw std::runtime_error("open file_2 failed\n");
    }

    if (!m_output_file){
        throw std::runtime_error("open output failed\n");
    }

    try {
        m_pcm_flt_buf = static_cast<uint8_t*>(m_mem_pool.allocate(m_pcm_flt_buf_size));
    } catch (const std::bad_alloc &e) {
        throw std::runtime_error("alloc pcm_flt_buf error: " + std::string(e.what()) + "\n");
    }

    try {
        m_pcm_s16_buf = static_cast<uint8_t*>(m_mem_pool.allocate(m_pcm_s16_buf_size));
    } catch (const std::bad_alloc &e) {
        throw std::runtime_error("alloc pcm_s16_buf error: " + std::string(e.what()) + "\n");
    }

    try {
        m_out_buf = static_cast<uint8_t*>(m_mem_pool.allocate(m_out_buf_size));
    } catch (const std::bad_alloc &e) {
        throw std::runtime_error("alloc " + std::string(e.what()) + "\n");
    }
}

void Mixer::DeConstruct() noexcept(true)
{
    if (m_pcm_flt_buf){
        m_mem_pool.deallocate(m_pcm_flt_buf,m_pcm_flt_buf_size);
        m_pcm_flt_buf = nullptr;
    }

    if (m_pcm_s16_buf){
        m_mem_pool.deallocate(m_pcm_s16_buf,m_pcm_s16_buf_size);
        m_pcm_s16_buf = nullptr;
    }

    if (m_out_buf){
        m_mem_pool.deallocate(m_out_buf,m_out_buf_size);
        m_out_buf = nullptr;
    }
}

Mixer::~Mixer(){
    DeConstruct();
}

Mixer_sp_type Mixer::create(const char **argv)
{
    Mixer_sp_type obj;
    try {
        obj = Mixer_sp_type(new Mixer(argv));
    } catch (const std::bad_alloc &e) {
        throw std::runtime_error("new Mixer error: " + std::string (e.what()) + "\n");
    }

    try {
        obj->Construct();
        return obj;
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw std::runtime_error("Mixer Construct error: " + std::string(e.what()) + "\n");
    }
}

void Mixer::exec() noexcept(false)
{
    size_t read_pcm_flt_size{},read_pcm_s16_size{};
    for(;;){




    }
}

Mixer_sp_type new_Mixer(const char **argv) noexcept(false)
{
    return Mixer::create(argv);
}


