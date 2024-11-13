#ifndef XAUDIOPLAY_HPP
#define XAUDIOPLAY_HPP

#include "xaduio_parameters.hpp"
#include "xsonic.hpp"

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
     * 支持ffmpeg参数,非必要不建议重写这两个函数
     * 如果用户重写这两个函数,若要支持播放倍速,请调用一次
     * virtual bool Open(const XAudioSpec &spec_);
     * @param parameters ffmpeg参数
     * @return true or false
     */
    virtual bool Open(const XCodecParameters &parameters) = 0;
    virtual bool Open(const XCodecParameters_sp &parameters) = 0;

    /**
     * 需被派生类继承,如果需要调整播放倍速,派生类继承本函数后,调用一次本函数
     * @param spec_ 音频相关参数
     * @return true or false
     */
    virtual bool Open(const XAudioSpec &spec_);

    void Push(const uint8_t * data, const size_t &size);
    void Push(const XAVFrame &);

    inline void set_volume(const int &volume) {
        m_volume_ = volume;
    }

    inline void set_speed(const double &s) {
        if(is_init_son_) {
            m_speed_ = static_cast<float>(s);
        }
    }

protected:
    explicit XAudio_Play() = default;

    static void AudioCallback(void *,uint8_t * ,int);

    virtual void Callback(uint8_t *,const int &) {}

    //bool Init_XSonic();

    std::mutex m_mux_;
    std::list<XAudio_Data> m_datum_;
    std::atomic_int m_volume_{128},
        m_format_size{-1};
    XAudioSpec m_spec_{};
private:
    std::atomic_bool is_init_son_{};
    std::atomic<float> m_speed_{1.0f};
    XSonic m_son;
};
class SDL_Audio;
#define xAudio() XAudio_Play::instance()

#endif
