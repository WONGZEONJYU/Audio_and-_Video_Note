#include "xaudio_play.hpp"
#include <SDL.h>
#include "xcodec_parameters.hpp"

using namespace std;

class SDL_Audio final : public XAudio_Play {

    atomic_int_fast64_t m_curr_pts_{};
    atomic_uint_fast64_t m_last_ms_{},
                    m_pause_begin_{};
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
        SDL_PauseAudio({});
        return true;
    }

    bool Open(const XCodecParameters &parameters) override {
        XAudioSpec spec;
        auto &[freq,format,
               channels,samples,fmt_size]{spec};

        freq = parameters.Sample_rate();
        channels = parameters.Ch_layout()->nb_channels;
        samples = parameters.Audio_nbSamples();
        format = ff_to_xaduio_format(parameters.Audio_sample_format());
        fmt_size = av_get_bytes_per_sample(static_cast<AVSampleFormat>(parameters.Audio_sample_format()));
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

    void Close() override {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        m_curr_pts_ = {};
        m_last_ms_ = {};
        unique_lock lock(m_mux_);
        m_datum_.clear();
    }

    void Pause(const bool &b) override{
        using XHelper::Get_time_ms;
        SDL_PauseAudio(b);
        if (b){
            m_pause_begin_ = Get_time_ms();
        }else{
//            if (m_pause_begin_ > 0){
//                m_last_ms_ += Get_time_ms() - m_pause_begin_;
//            }
            m_last_ms_ += Get_time_ms() - m_pause_begin_;
        }
    }

    auto curr_pts() ->int64_t override {
        double diff_ms{};
        if (m_last_ms_ > 0) {
            diff_ms = static_cast<decltype(diff_ms)>(XHelper::Get_time_ms() - m_last_ms_);
        }

        //cerr << __FUNCTION__ << " diff_ms = " << diff_ms << "\n";
        if (m_time_base_ > 0) {
            diff_ms = diff_ms / 1000.0 * m_time_base_;
        }

        const auto pts{m_curr_pts_ + static_cast<int64_t>(diff_ms / m_speed_)};
        //cerr << __FUNCTION__ << " pts = " << pts << "\n";
        return pts;
    }

private:
    void Callback(uint8_t * const stream,const int &len) override {

        fill_n(stream,len,0);

        uint64_t need_size{static_cast<decltype(need_size)>(len)}, //需要处理的大小
                mixed_size{}; //已经处理的数据的大小

        unique_lock locker(m_mux_);
        if (m_datum_.empty()) {
            return;
        }

        m_last_ms_ = XHelper::Get_time_ms();
        m_curr_pts_ = m_datum_.front().m_pts;//当前播放的PTS

        while (mixed_size < len) {

            if (m_datum_.empty()) {
                break;
            }

            auto &[buf,offset,pts]{m_datum_.front()};
            auto size{buf.size() - offset};
            //auto size{out.size() - in.m_offset};

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
