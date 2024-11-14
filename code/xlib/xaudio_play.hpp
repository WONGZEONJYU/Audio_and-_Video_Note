#ifndef XAUDIOPLAY_HPP
#define XAUDIOPLAY_HPP

#include "xaduio_parameters.hpp"
#include "xaudio_triple_speed.hpp"
#include "xswrsample.hpp"

struct XAudio_Data {
    std::vector<uint8_t> m_data{};
    std::atomic_uint64_t m_offset{};
};

class XLIB_API XAudio_Play {
    X_DISABLE_COPY_MOVE(XAudio_Play)
    using data_buffer_t = std::vector<uint8_t>;
    int64_t Speed_Change(data_buffer_t &,data_buffer_t &);

    void push_helper(data_buffer_t &);

public:
    static XAudio_Play *instance();
    virtual ~XAudio_Play() = default;

    /**
     * 如果需要支持ffmpeg接口,则重载以下两个函数或其中一个
     * @param parameters ffmpeg参数
     * @return true or false
     */
    virtual auto Open(const XCodecParameters &parameters)->bool {return {};}
    virtual auto Open(const XCodecParameters_sp &parameters) ->bool {return {};}

    /**
     * @param spec_ 音频相关参数
     * @return true or false
     */
    virtual bool Open(const XAudioSpec &spec_) = 0;

    void Push(const uint8_t * data, const size_t &size);
    void Push(const XAVFrame &);

    inline void set_volume(const int &volume) {
        m_volume_ = volume;
    }

    inline void set_speed(const double &s) {
        if(UNKNOWN_ != m_format_size_) {
            m_speed_ = static_cast<float>(s);
        }
    }

protected:

    enum FMT_SIZE: int {
        UNKNOWN_ = -1,
        U8S8_ = sizeof(char),
        U16S16_ = sizeof(short),
        U32S32_ = sizeof(int),
        FLOAT_ = sizeof(float),
        DOUBLE_ = sizeof(double),
    };

    explicit XAudio_Play() = default;

    static void AudioCallback(void *,uint8_t * ,int);

    virtual void Callback(uint8_t *,const int &) {}

    bool support_gear_shift(const XAudioSpec &spec_,const FMT_SIZE &fmt_size);

    std::mutex m_mux_;
    std::list<XAudio_Data> m_datum_;
    std::atomic_int m_volume_{128};
    XAudioSpec m_spec_{};

private:
    std::atomic<FMT_SIZE> m_format_size_{UNKNOWN_};
    std::atomic<float> m_speed_{1.0f};
    Audio_Playback_Speed m_speed_ctr_;
    XSwrSample_sp m_swr_;
};

#define xAudio() XAudio_Play::instance()

#endif
