#ifndef FLVMETADATA_H
#define FLVMETADATA_H

#include <cstdint>

class FlvMetaData
{
    void Shallow_copy(const FlvMetaData&)noexcept;
    static void _Clear_Rvalue(FlvMetaData&&) noexcept;
    void _Copy_construct_from(const FlvMetaData&) noexcept;
    void _Move_construct_from(FlvMetaData&&) noexcept;
    void _Swap(FlvMetaData &);
    FlvMetaData() = default;
public:
    FlvMetaData(const uint8_t *, uint32_t );
    ~FlvMetaData();

    FlvMetaData(const FlvMetaData&);
    FlvMetaData(FlvMetaData&&) noexcept ;
    FlvMetaData& operator=(const FlvMetaData&);
    FlvMetaData& operator=(FlvMetaData&&) noexcept;

    [[nodiscard]] double getDuration() const;
    [[nodiscard]] double getWidth() const;
    [[nodiscard]] double getHeight() const;
    [[nodiscard]] double getFramerate() const;
    [[nodiscard]] double getVideoDatarate() const;
    [[nodiscard]] double getAudioDatarate() const;
    [[nodiscard]] double getVideoCodecId()const;
    [[nodiscard]] double getAudioCodecId()const;
    [[nodiscard]] double getAudioSamplerate()const;
    [[nodiscard]] double getAudioSamplesize()const;
    [[nodiscard]] bool getStereo()const;

private:
    //convert HEX to double
    static double hexStr2double(const uint8_t* , uint32_t = 8);
    void parseMeta();

    uint8_t *m_meta{};
    uint32_t m_length{};
    double m_duration{},
            m_width{},
            m_height{},
            m_framerate{},
            m_videodatarate{},
            m_audiodatarate{},
            m_videocodecid{},
            m_audiocodecid{},
            m_audiosamplerate{},
            m_audiosamplesize{};
    bool m_stereo{};
};

#endif // FLVMETADATA_H
