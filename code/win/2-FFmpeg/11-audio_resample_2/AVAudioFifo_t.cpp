
extern "C"{
#include <libavutil/fifo.h>
}

#include "AVAudioFifo_t.h"

#include <iostream>

bool AVAudioFifo_t::construct(const AVSampleFormat &sample_fmt,
             const int& channels,const int &nb_samples) {

    m_audio_fifo = av_audio_fifo_alloc(sample_fmt,channels,nb_samples);
    return m_audio_fifo;
}

AVAudioFifo_t::AVAudioFifo_sp_t AVAudioFifo_t::create(const AVSampleFormat &sample_fmt,
                                                        const int& channels,const int &nb_samples) noexcept(false){
    try {
        AVAudioFifo_sp_t obj(new AVAudioFifo_t);
        if(!obj->construct(sample_fmt,channels,nb_samples)) {
            obj.reset();
            throw std::runtime_error("av_audio_fifo_alloc failed\n");
        }
        return obj;
    } catch (const std::bad_alloc &e) {
        std::cerr << e.what() << "\n";
        throw std::runtime_error("AVAudioFifo_t construct failed\n");
    }
}

int AVAudioFifo_t::write(void * const * data,const int& nb_samples) const{
    return av_audio_fifo_write(m_audio_fifo,data,nb_samples);
}

int AVAudioFifo_t::read(void * const * data,const int &nb_samples) const{
    return av_audio_fifo_read(m_audio_fifo,data,nb_samples);
}

int AVAudioFifo_t::size() const
{
    return av_audio_fifo_size(m_audio_fifo);
}

AVAudioFifo_t::~AVAudioFifo_t() {
    av_fifo_freep2(reinterpret_cast<AVFifo **>(&m_audio_fifo));
}


