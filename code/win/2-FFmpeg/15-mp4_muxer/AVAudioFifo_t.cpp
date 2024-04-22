
extern "C"{
#include <libavutil/fifo.h>
}

#include "AVAudioFifo_t.h"
#include "AVHelper.h"

AVAudioFifo_t::AVAudioFifo_t(const AVSampleFormat &sample_fmt,
              const int& channels,const int &nb_samples) noexcept(true):
        m_audio_fifo(av_audio_fifo_alloc(sample_fmt,channels,nb_samples))
{
}

void AVAudioFifo_t::Construct() const noexcept(false){

    if (!m_audio_fifo){
        throw std::runtime_error("av_audio_fifo_alloc failed\n");
    }
}

AVAudioFifo_sp_type AVAudioFifo_t::create(const AVSampleFormat &sample_fmt,
                                          const int& channels,const int &nb_samples) noexcept(false)
{
    AVAudioFifo_sp_type obj;

    try {
        obj = std::move(AVAudioFifo_sp_type(new AVAudioFifo_t(sample_fmt,channels,nb_samples)));
    }catch (const std::bad_alloc &e){
        throw std::runtime_error("new AVAudioFifo_t failed: " + std::string (e.what()) + "\n");
    }

    try {
        obj->Construct();
        return obj;
    }catch (const std::runtime_error &e){
        obj.reset();
        throw std::runtime_error("AVAudioFifo_t construct failed: " + std::string(e.what()) + "\n");
    }
}

int AVAudioFifo_t::write(void * const * data,const int& nb_samples) const{

    const auto ret{av_audio_fifo_write(m_audio_fifo,data,nb_samples)};
    if (ret < 0){
        throw std::runtime_error("Audio_FIFO write failed: " + AVHelper::av_get_err(ret) + "\n");
    }
    return ret;
}

int AVAudioFifo_t::read(void * const * data,const int &nb_samples) const{

    const auto ret{av_audio_fifo_read(m_audio_fifo,data,nb_samples)};
    if (ret < 0){
        throw std::runtime_error("Audio_FIFO read failed: " + AVHelper::av_get_err(ret) + "\n");
    }
    return ret;
}

int AVAudioFifo_t::size() const noexcept(true){
    return av_audio_fifo_size(m_audio_fifo);
}

void AVAudioFifo_t::DeConstruct() noexcept(true) {
    av_fifo_freep2(reinterpret_cast<AVFifo **>(&m_audio_fifo));
}

AVAudioFifo_t::~AVAudioFifo_t() {
    DeConstruct();
}
