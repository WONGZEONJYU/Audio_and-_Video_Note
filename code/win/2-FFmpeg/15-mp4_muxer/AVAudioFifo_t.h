#ifndef AVAUDIOFIFO_T_H
#define AVAUDIOFIFO_T_H

extern "C"{
#include <libavutil/audio_fifo.h>
}

#include <memory>

class AVAudioFifo_t final {

    explicit AVAudioFifo_t(const AVSampleFormat &sample_fmt,
                           const int& channels,const int &nb_samples) noexcept(true);
    void Construct() const noexcept(false);
    void DeConstruct() noexcept(true);
public:
    using AVAudioFifo_sp_t = std::shared_ptr<AVAudioFifo_t>;
    AVAudioFifo_t(const AVAudioFifo_t&) = delete;
    AVAudioFifo_t& operator=(const AVAudioFifo_t&) = delete;
    static AVAudioFifo_sp_t create(const AVSampleFormat &sample_fmt,
         const int& channels,
         const int &nb_samples = 1) noexcept(false);

    ~AVAudioFifo_t();
    int write(void * const *,const int& ) const noexcept(false);
    int read(void * const *,const int& ) const noexcept(false);
    [[nodiscard]] int size() const noexcept(true);

private:
    AVAudioFifo *m_audio_fifo{};
};

using AVAudioFifo_sp_type = typename AVAudioFifo_t::AVAudioFifo_sp_t;

AVAudioFifo_sp_type new_AVAudioFifo_t(const AVSampleFormat &sample_fmt,
                                      const int& channels,
                                      const int &nb_samples = 1);

#endif
