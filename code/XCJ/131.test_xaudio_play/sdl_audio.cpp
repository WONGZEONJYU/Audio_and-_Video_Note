#include "xaudio_play.hpp"
#include  <SDL.h>

using namespace std;

class SDL_Audio final : public XAudio_Play {

public:
    explicit SDL_Audio() {
        SDL_Init(SDL_INIT_AUDIO);
    }

    ~SDL_Audio() override {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }

    bool Open(const XAudioSpec &spec_) override  {

        SDL_QuitSubSystem(SDL_INIT_AUDIO);

        SDL_AudioSpec spec{
            .freq = spec_.m_freq,
            .format = spec_.m_format,
            .channels = spec_.m_channels,
            .silence = 0,
            .samples = spec_.m_samples,
            .callback = AudioCallback,
            .userdata = this
        };
        SDL2_INT_ERR_OUT(SDL_OpenAudio(&spec,{}),return {});
        CHECK_FALSE_(XAudio_Play::Open(spec_),return {});
        SDL_PauseAudio(0);
        m_spec_ = spec_;
        return true;
    }

    void Callback(uint8_t * const stream, const int &len) override {

        std::fill_n(stream,len, 0);

        unique_lock locker(m_mux_);

        if (m_datum_.empty()) {
            return;
        }

        uint64_t need_size{static_cast<decltype(need_size)>(len)}, //需要处理的大小
                mixed_size{}; //已经处理的数据的大小

        while (mixed_size < len) {

            if (m_datum_.empty()) {
                break;
            }

            auto &[buf,offset]{m_datum_.front()};
            auto size{buf.size() - offset};

            if (size > need_size) {
                size = need_size;
            }

            SDL_MixAudio(stream + mixed_size,buf.data() + offset,size,m_volume_);

            need_size -= size;
            mixed_size += size;
            offset += size;
            if (offset >= buf.size()) {
                m_datum_.pop_front();
            }
        }
    }
};

XAudio_Play * XAudio_Play::instance() {
    static SDL_Audio audio;
    return &audio;
}
