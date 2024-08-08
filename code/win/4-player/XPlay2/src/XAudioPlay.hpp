//
// Created by Administrator on 2024/8/6.
//

#ifndef XPLAY2_XAUDIOPLAY_HPP
#define XPLAY2_XAUDIOPLAY_HPP

class XAudioPlay {

public:
    virtual ~XAudioPlay() = default;
    virtual void Open() = 0;
    virtual void Close() = 0;
    virtual void Write(const unsigned char *,const long long &) = 0;
    [[nodiscard]] virtual unsigned long long FreeSize() const {return 0;}
    [[nodiscard]] virtual unsigned long long BufferSize() const {return 0;}
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
                             const int &SampleFormat = 2) noexcept(true) ;

protected:
    int m_SampleRate{44100},
    /**
     * m_SampleFormat取决于使用的库,此处默认为QT库的值
     */
        m_SampleFormat{2},
        m_Channels{2};
};

#endif
