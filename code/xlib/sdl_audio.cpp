#include "xaudio_play.hpp"
#include  <SDL.h>
#include "xcodec_parameters.hpp"

using namespace std;



class XLIB_API SDL_Audio final : public XAudio_Play {

public:
    explicit SDL_Audio() {
        SDL_Init(SDL_INIT_AUDIO);
    }

    ~SDL_Audio() override {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }

    bool Open(const XAudioSpec &spec_) override {


        SDL_QuitSubSystem(SDL_INIT_AUDIO);

        SDL_AudioSpec spec{};
        auto &[freq,format,channels,
               silence,samples,padding,
               size,callback,userdata]{spec};

        freq = spec_.m_freq;
        format = spec_.m_format;
        channels = spec_.m_channels;
        samples = spec_.m_samples;
        callback = AudioCallback;
        userdata = this;

        int ret;
        SDL2_INT_ERR_OUT(ret = SDL_OpenAudio(&spec,{}));
        if (ret < 0) {
            SDL_AudioSpec default_spec{
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

            SDL2_INT_ERR_OUT(ret = SDL_OpenAudio(&default_spec,{}),return {});
        }

        CHECK_FALSE_(support_gear_shift(spec_,FLOAT_),return {});
        SDL_PauseAudio(0);
        return true;
    }

    bool Open(const XCodecParameters &parameters) override {
        XAudioSpec spec;
        auto &[freq,format,channels
                ,samples]{spec};

        freq = parameters.Sample_rate();
        channels = parameters.Ch_layout()->nb_channels;

        const auto frame_size{parameters.Audio_nbSamples()};
        samples = frame_size > 0 ? frame_size : samples;

        switch (parameters.Audio_sample_format()) {
            case AV_SAMPLE_FMT_U8:
            case AV_SAMPLE_FMT_U8P: {
                format = AUDIO_S8;
                break;
            }
            case AV_SAMPLE_FMT_S16:         ///< signed 16 bits
            case AV_SAMPLE_FMT_S16P: {      ///< signed 16 bits, planar
                format = AUDIO_S16;
                break;
            }
            case AV_SAMPLE_FMT_S32:         ///< signed 32 bits
            case AV_SAMPLE_FMT_S32P: {      ///< signed 32 bits, planar
                format = AUDIO_S32;
                break;
            }
            case AV_SAMPLE_FMT_FLT:         ///< float
            case AV_SAMPLE_FMT_FLTP: {      ///< float, planar
                format = AUDIO_F32;
                break;
            }
            default:
                format = -1;
                break;
        }

        return Open(spec);
    }

    bool Open(const XCodecParameters_sp &parameters) override{
        if (!parameters){
            PRINT_ERR_TIPS(GET_STR(parameters empty!));
            return {};
        }
        return Open(*parameters);
    }
private:
    void Callback(uint8_t * const stream, const int &len) override {

        std::fill_n(stream,len,0);
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

XAudio_Play * XAudio_Play::instance() {
    static SDL_Audio audio;
    return &audio;
}
