#ifndef SONIC_HPP
#define SONIC_HPP

#include "xhelper.hpp"

class XSonic_data {
    friend class XSonic;
protected:
    XSonic_data() = default;
    static inline constexpr auto SONIC_MIN_PITCH_{65},
            SONIC_MAX_PITCH_{400},
            SONIC_AMDF_FREQ_{4000};

    float m_speed{},m_volume{},m_pitch{},m_rate{},m_avePower{};
    int m_oldRatePosition{},m_newRatePosition{},m_useChordPitch{},m_quality{},
            m_numChannels{},m_inputBufferSize{},m_pitchBufferSize{},m_outputBufferSize{},
            m_numInputSamples{},m_numOutputSamples{},m_numPitchSamples{},m_minPeriod{},
            m_maxPeriod{},m_maxRequired{},m_remainingInputToCopy{},m_sampleRate{},
            m_prevPeriod{},m_prevMinDiff{};
    bool m_is_init_{};
};

class XLIB_API XSonic final: XSonic_data {

    static void scaleSamples(int16_t * ,
                             const int&,
                             const float &);

    bool allocateStreamBuffers(const int &sampleRate,
                               const int &numChannels);

    bool sonicCreateStream(const int &sampleRate,
                           const int &numChannels);

    bool enlargeOutputBufferIfNeeded(const int &numSamples);

    bool enlargeInputBufferIfNeeded(const int &numSamples);

    bool addDoubleSamplesToInputBuffer(const double *samples,
                                      const int &numSamples);

    bool addS64SamplesToInputBuffer(const int64_t *samples,
                                      const int &numSamples);

    bool addU64SamplesToInputBuffer(const uint64_t *samples,
                                  const int &numSamples);

    bool addFloatSamplesToInputBuffer(const float *samples,
                                      const int &numSamples);

    bool addS32SamplesToInputBuffer(const int32_t *samples,
                                    const int &numSamples);

    bool addU32SamplesToInputBuffer(const uint32_t *samples,
                                    const int &numSamples);

    bool addShortSamplesToInputBuffer(const int16_t *samples,
                                      const int &numSamples);

    bool addUnsignedShortSamplesToInputBuffer(const uint16_t *samples,
                                            const int &numSamples);

    bool addCharSamplesToInputBuffer(const int8_t *samples,
                                    const int &numSamples);

    bool addUnsignedCharSamplesToInputBuffer(const uint8_t *samples,
                                             const int &numSamples);

    void removeInputSamples(const int &position);

    bool copyToOutput(const int16_t *samples,
                      const int &numSamples);

    int copyInputToOutput(const int &position);

    void downSampleInput(const int16_t *samples,
                         const int &skip);

    static int findPitchPeriodInRange(const int16_t *samples,
                                      const int &minPeriod,
                                      const int &maxPeriod,
                                      int &retMinDiff,
                                      int &retMaxDiff);

    [[nodiscard]] bool prevPeriodBetter(const int &minDiff,
                                        const int &maxDiff,
                                        const int &preferNewPeriod) const;

    int findPitchPeriod(const int16_t *samples,const int &preferNewPeriod);

    static void overlapAdd(const int &numSamples,
                           const int &numChannels,
                           int16_t *out,
                           const int16_t *rampDown,
                           const int16_t *rampUp);

    static void overlapAddWithSeparation(const int &numSamples,
                                         const int &numChannels,
                                         const int &separation,
                                         int16_t *out,
                                         const int16_t *rampDown,
                                         const int16_t *rampUp);

    bool moveNewSamplesToPitchBuffer(const int &originalNumOutputSamples);

    void removePitchSamples(const int &numSamples);

    bool adjustPitch(const int &originalNumOutputSamples);

    static int findSincCoefficient(const int &i,
                                   const int &ratio,
                                   const int &width);

    int16_t interpolate(const int16_t *in,
                        const int &oldSampleRate,
                        const int &newSampleRate) const;

    bool adjustRate(const float &rate,
                    const int &originalNumOutputSamples);

    int skipPitchPeriod( const int16_t *samples,
                         const float &speed,
                         const int &period);

    int insertPitchPeriod(const int16_t *samples,
                          const float &speed,
                          const int &period);

    bool changeSpeed(const float &speed);

    bool processStreamInput();

    void Move_(XSonic *) noexcept(true);

public:
    bool Open(const int &sampleRate,const int &numChannels);
    void Close();

    bool sonicWriteDoubleToStream(const double *samples,const int &numSamples);

    /* Use this to write floating point data to be speed up or down into the stream.
   Values must be between -1 and 1.  Return 0 if memory realloc failed, otherwise 1 */
    bool sonicWriteFloatToStream(const float *samples,const int &numSamples);
/* Use this to write 16-bit data to be speed up or down into the stream.
   Return 0 if memory realloc failed, otherwise 1 */

    bool sonicWriteU64ToStream(const uint64_t *samples,const int &numSamples);

    bool sonicWriteS64ToStream(const int64_t *samples,const int &numSamples);

    bool sonicWriteS32ToStream(const int32_t *samples,const int &numSamples);

    bool sonicWriteU32ToStream(const uint32_t *samples,const int &numSamples);

    bool sonicWriteShortToStream(const int16_t *samples,const int &numSamples);
/* Use this to write 8-bit unsigned data to be speed up or down into the stream.
   Return 0 if memory realloc failed, otherwise 1 */

    bool sonicWriteUnsignedShortToStream(const uint16_t *samples,const int &numSamples);

    bool sonicWriteCharToStream(const int8_t *samples,const int &numSamples);

    bool sonicWriteUnsignedCharToStream(const uint8_t *samples,const int &numSamples);

    int sonicReadDoubleFromStream(double *samples,const int &maxSamples);

    int sonicReadS64FromStream(int64_t *samples,const int &maxSamples);

    int sonicReadU64FromStream(uint64_t *samples,const int &maxSamples);

/* Use this to read floating point data out of the stream.  Sometimes no data
   will be available, and zero is returned, which is not an error condition. */
    int sonicReadFloatFromStream(float *samples,const int &maxSamples);

    int sonicReadU32FromStream(uint32_t *samples,const int &maxSamples);

    int sonicReadS32FromStream(int32_t *samples,const int &maxSamples);

/* Use this to read 16-bit data out of the stream.  Sometimes no data will
   be available, and zero is returned, which is not an error condition. */
    int sonicReadShortFromStream(int16_t *samples,const int &maxSamples);

    int sonicReadUnsignedShortFromStream(uint16_t *samples,const int &maxSamples);

/* Use this to read 8-bit unsigned data out of the stream.  Sometimes no data will
   be available, and zero is returned, which is not an error condition. */
    int sonicReadUnsignedCharFromStream(uint8_t *samples,const int &maxSamples);

    int sonicReadSignedCharFromStream(int8_t *samples,const int &maxSamples);

/* Force the sonic stream to generate output using whatever data it currently
   has.  No extra delay will be added to the output, but flushing in the middle of
   words could introduce distortion. */
    bool sonicFlushStream();
/* Return the number of samples in the output buffer */
    [[nodiscard]] int sonicSamplesAvailable() const;
/* Get the speed of the stream. */
    [[nodiscard]] float sonicGetSpeed() const;
/* Set the speed of the stream. */
    void sonicSetSpeed(const float &speed);
/* Get the pitch of the stream. */
    [[nodiscard]] float sonicGetPitch() const;
/* Set the pitch of the stream. */
    void sonicSetPitch(const float &pitch);
/* Get the rate of the stream. */
    [[nodiscard]] float sonicGetRate() const;
/* Set the rate of the stream. */
    void sonicSetRate(const float &rate);
/* Get the scaling factor of the stream. */
    [[nodiscard]] float sonicGetVolume() const;
/* Set the scaling factor of the stream. */
    void sonicSetVolume(const float &volume);
/* Get the chord pitch setting. */
    [[nodiscard]] int sonicGetChordPitch() const;
/* Set chord pitch mode on or off.  Default is off.  See the documentation
   page for a description of this feature. */
    void sonicSetChordPitch(const int &useChordPitch);
/* Get the quality setting. */
    [[nodiscard]] int sonicGetQuality() const;
/* Set the "quality".  Default 0 is virtually as good as 1, but very much faster. */
    void sonicSetQuality(const int &quality);
/* Get the sample rate of the stream. */
    [[nodiscard]] int sonicGetSampleRate() const;
/* Get the number of channels. */
    [[nodiscard]] int sonicGetNumChannels() const;

private:
    std::vector<int16_t> m_inputBuffer,m_outputBuffer,
                        m_pitchBuffer,m_downSampleBuffer;
public:
    X_DISABLE_COPY(XSonic)
    explicit XSonic() = default;
    explicit XSonic(const int &sampleRate,const int &numChannels);
    XSonic(XSonic &&) noexcept(true);
    XSonic &operator=(XSonic &&) noexcept(true);
    ~XSonic() = default;
};

/* This is a non-stream oriented interface to just change the speed of a sound
   sample.  It works in-place on the sample array, so there must be at least
   speed*numSamples available space in the array. Returns the new number of samples. */
XLIB_API int sonicChangeFloatSpeed(float *samples,
                          const int &numSamples,
                          const float &speed = 1.0f,
                          const float &pitch = 1.0f,
                          const float &rate = 1.0f,
                          const float &volume = 1.0f,
                          const int &useChordPitch = 0,
                          const int &sampleRate = 44100,
                          const int &numChannels = 2);

/* This is a non-stream oriented interface to just change the speed of a sound
   sample.  It works in-place on the sample array, so there must be at least
   speed*numSamples available space in the array. Returns the new number of samples. */
XLIB_API int sonicChangeShortSpeed(int16_t *samples,
                          const int &numSamples,
                          const float &speed = 1.0f,
                          const float &pitch = 1.0f,
                          const float &rate = 1.0f,
                          const float &volume = 1.0f,
                          const int &useChordPitch = 0,
                          const int &sampleRate = 44100,
                          const int &numChannels = 2);
#endif
