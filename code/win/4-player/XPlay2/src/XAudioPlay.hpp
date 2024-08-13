//
// Created by Administrator on 2024/8/6.
//

#ifndef XPLAY2_XAUDIOPLAY_HPP
#define XPLAY2_XAUDIOPLAY_HPP

#include <atomic>

class XAudioPlay {

public:
    virtual ~XAudioPlay() = default;
    virtual void Open() = 0;
    virtual void Close() = 0;

    virtual void Write(const unsigned char *,const long long &) noexcept(false) = 0;

    /**
     * 从设备获取空闲大小,即未播放的数据
     * @return
     */
    [[nodiscard]] virtual unsigned long long FreeSize() const {return 0;}

    /**
     * 从设备获取一帧音频的大小
     * @return
     */
    [[nodiscard]] virtual unsigned long long BufferSize() const {return 0;}

    /**
     * @return 未播放的时间
     */

    [[nodiscard]] virtual long long NoPlayMs() const = 0;
    /**
     * QT专用,非QT使用无效
     */
    virtual void QtSetParent(void *) noexcept(true){};
    /**
     * SampleFormat 需要根据库去决定,这里使用qt库,QAudioFormat::Int16的值是2
     * @param SampleRate
     * @param Channels
     * @param SampleFormat
     */
    void set_Audio_parameter(const int &SampleRate,
                             const int &Channels,
                             const int &SampleFormat = 2) noexcept(true) {
        m_SampleRate = SampleRate;
        m_SampleFormat = SampleFormat;
        m_Channels = Channels;
        m_is_change = true;
    }

    [[nodiscard]] auto Is_Transform() noexcept(true){
        return m_is_change.load();
    }

protected:
    std::atomic_bool m_is_change{};
    std::atomic_int m_SampleRate{44100},
    /**
     * m_SampleFormat取决于使用的库,此处默认为QT库的值
     */
        m_SampleFormat{2},
        m_Channels{2};
};

#endif
