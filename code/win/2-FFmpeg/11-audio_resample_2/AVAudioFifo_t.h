#ifndef AVAUDIOFIFO_T_H
#define AVAUDIOFIFO_T_H

extern "C"{
#include <libavutil/audio_fifo.h>
}

#include <memory>

class AVAudioFifo_t final{

    explicit AVAudioFifo_t() = default;
    AVAudioFifo *m_audio_fifo{};
    bool construct(const AVSampleFormat &sample_fmt,
         const int& channels,
         const int &nb_samples);

public:
    using AVAudioFifo_sp_t = std::shared_ptr<AVAudioFifo_t>;

    static AVAudioFifo_sp_t create(const AVSampleFormat &sample_fmt,
         const int& channels,
         const int &nb_samples) noexcept(false);

    AVAudioFifo_t(const AVAudioFifo_t&) = delete;
    AVAudioFifo_t& operator=(const AVAudioFifo_t&) = delete;
    ~AVAudioFifo_t();
    int write(void * const *,const int& ) const;
    int read(void * const *,const int& ) const;
    [[nodiscard]] int size() const;
};

#endif //AVAUDIOFIFO_T_H
