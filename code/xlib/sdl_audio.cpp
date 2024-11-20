#include "xaudio_play.hpp"
#include <SDL.h>
#include "xcodec_parameters.hpp"

using namespace std;

class SDL_Audio final : public XAudio_Play {

public:
    explicit SDL_Audio() {
        SDL_Init(SDL_INIT_AUDIO);
    }

    ~SDL_Audio() override {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }

    bool Open(const XAudioSpec &spec_) override {

        const SDL_AudioSpec default_spec{
            .freq = 44100,
            .format = AUDIO_S16SYS,
            .channels = 2,
            .silence = 0,
            .samples = 1024,
            .padding = 0,
            .size = 0,
            .callback = AudioCallback,
            .userdata = this,
        };

        SDL_QuitSubSystem(SDL_INIT_AUDIO); //用于关闭回调函数

        SDL_AudioSpec spec{};
        auto &[freq,format,channels,
               silence,samples,padding,
               size,callback,userdata]{spec};

        freq = spec_.m_freq;
        format = to_sdl_audio_format(spec_.m_format);
        channels = spec_.m_channels;
        samples = spec_.m_samples;
        callback = AudioCallback;
        userdata = this;

        int ret;
        SDL2_INT_ERR_OUT(ret = SDL_OpenAudio(&spec,{}));
        m_spec_ = spec_;
        if (ret < 0) { //用默认参数进行初始化
            auto tmp_spec{default_spec};
            SDL2_INT_ERR_OUT(ret = SDL_OpenAudio(&tmp_spec,{}),return {});
            m_spec_.m_channels = tmp_spec.channels;
            m_spec_.m_samples = tmp_spec.samples;
            m_spec_.m_freq = tmp_spec.freq;
            m_spec_.m_format = sdl_to_xaudio_format(tmp_spec.format);
        }

        CHECK_FALSE_(init_speed_ctr(m_spec_.m_freq,m_spec_.m_channels),return {});
        SDL_PauseAudio(0);
        return true;
    }

    bool Open(const XCodecParameters &parameters) override {
        XAudioSpec spec;
        auto &[freq,format,channels,samples]{spec};

        freq = parameters.Sample_rate();
        channels = parameters.Ch_layout()->nb_channels;
        samples = parameters.Audio_nbSamples();
        format = ff_to_xaduio_format(parameters.Audio_sample_format());
        const auto b{Open(spec)};
        if (b) {
            XAudio_Play::Open(parameters); //复制一份参数
        }
        return b;
    }

    bool Open(const XCodecParameters_sp &parameters) override {
        if(!parameters) {
            PRINT_ERR_TIPS(GET_STR(parameters error!));
            return {};
        }
        return Open(*parameters);
    }

private:
    void Callback(uint8_t * const stream, const int &len) override {

        fill_n(stream,len,0);
        uint64_t need_size{static_cast<decltype(need_size)>(len)}, //需要处理的大小
                mixed_size{}; //已经处理的数据的大小

        while (mixed_size < len) {
            unique_lock locker(m_mux_);
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

XAudio_Play *XAudio_Play::instance() {
    static SDL_Audio audio;
    return &audio;
}
