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
public:
    static XAudio_Play *instance();

    virtual ~XAudio_Play() = default;

    virtual bool Open(const XAudioSpec &spec_) {
        const auto r{m_son.Open(spec_.m_freq,spec_.m_channels)};
        if (r) {
            m_spec_ = spec_;
        }
        return r;
    }

    void Push(const uint8_t * data, const size_t &size);

    void set_volume(const int &volume) {
        m_volume_ = volume;
    }

    void set_speed(const double &s) {
        m_speed_ = static_cast<float>(s);
    }

protected:
    explicit XAudio_Play() = default;

    static void AudioCallback(void *,uint8_t * ,int);

    virtual void Callback(uint8_t *,const int &) {}

    std::mutex m_mux_;
    std::list<XAudio_Data> m_datum_;
    std::atomic_int m_volume_{128};
    XAudioSpec m_spec_{};
private:
    std::atomic<float> m_speed_{1.0f};
    XSonic m_son;
};

#define xAudio() XAudio_Play::instance()

#endif
