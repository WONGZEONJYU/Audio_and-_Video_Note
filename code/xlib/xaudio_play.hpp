#ifndef XAUDIOPLAY_HPP
#define XAUDIOPLAY_HPP

#include "xaduio_parameters.hpp"
#include "xaudio_triple_speed.hpp"
#include "xcodec_parameters.hpp"

struct XAudio_Data {
    std::vector<uint8_t> m_data{};
    uint64_t m_offset{};
    int64_t m_pts{};
};

struct XSwrParam;

class XLIB_API XAudio_Play {
    X_DISABLE_COPY_MOVE(XAudio_Play)
protected:
    using data_buffer_t = std::vector<uint8_t>;
private:
    void push_helper(data_buffer_t &,const int64_t &pts);

public:
    static XAudio_Play *instance();
    virtual ~XAudio_Play() = default;

    /**
     * 如果需要支持ffmpeg接口,则重载以下两个函数或其中一个
     * 用户重载后,可以调用本函数对ffmpeg的参数进行copy
     * @param parameters ffmpeg参数
     * @return true or false
     */
    virtual auto Open(const XCodecParameters &parameters)->bool;
    virtual auto Open(const XCodecParameters_sp &parameters) ->bool;

    /**
     * @param spec_ 音频相关参数
     * @return true or false
     */
    virtual bool Open(const XAudioSpec &spec_) = 0;
    virtual void Close() = 0;
    virtual void Clear() {
        Close();
        Open(m_spec_);
    }

    /**
     * PCM裸数据,可以是平面格式或交叉模式
     * 如果播放库不支持平面格式,可能有奇奇怪怪的问题
     * @param data
     * @param size
     * @param pts
     */
    void Push(const uint8_t *data,
        const size_t &size,
        const int64_t &pts);

    /**
     * 直接支持ffmpeg接口
     * @param frame
     */
    void Push(const XAVFrame &frame);

    [[maybe_unused]] inline void set_volume(const int &volume) {
        m_volume_ = volume;
    }

    /**
     * 设置音频速度
     * @param s 速度范围不建议太大
     */
    [[maybe_unused]] void set_speed(const double &s) ;

    /**
     * 本函数用于音频同步视频
     * 获取音频PTS + 带上时间差PTS
     * @return pts + ms / time_base
     */
    virtual auto curr_pts() ->int64_t = 0;

    /**
     * 设置时间基准
     * @param time_base
     */
    void set_time_base(const double &time_base) {
        m_time_base_ = time_base;
    }

    /**
     * 暂停
     */
    virtual void Pause(const bool &) = 0;

protected:
    explicit XAudio_Play() = default;

    static void AudioCallback(void *,uint8_t * ,int);

    virtual void Callback(uint8_t *,const int &) {}

    int64_t Speed_Change(data_buffer_t &,data_buffer_t &);
    /**
     * 重采样初始化,本库暂时没有使用到
     * @return ture or false
     */
    [[maybe_unused]] bool init_swr(const XSwrParam &);

    [[maybe_unused]] bool init_speed_ctr(const int &sample_rate,
                                         const int &channels);
protected:
    std::mutex m_mux_;
    std::list<XAudio_Data> m_datum_;
    std::atomic_int m_volume_{128};
    XAudioSpec m_spec_{};
    std::atomic<double> m_time_base_{};
    std::atomic<float> m_speed_{1.0f};

private:
    Audio_Playback_Speed m_speed_ctr_;
    XSwrSample_sp m_swr_;
    XCodecParameters m_ff_audio_parameters_;
    std::atomic_bool m_init_speed_ctr_{};
};

#define xAudio() XAudio_Play::instance()

#endif
