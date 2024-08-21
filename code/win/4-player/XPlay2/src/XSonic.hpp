#ifndef SONIC_HPP
#define SONIC_HPP

#include "XHelper.hpp"
#include <vector>

class XSonic {
    static inline constexpr auto SONIC_MIN_PITCH_ {65},
                                SONIC_MAX_PITCH_ {400},
                                SONIC_AMDF_FREQ_ {4000};

    bool enlargeOutputBufferIfNeeded(const int&);
    bool copyToOutput(const int16_t *,const int &);
    int copyInputToOutput(const int &);
    static int findPitchPeriodInRange(const int16_t *,const int &,
                               const int &,int &,int &);
    void downSampleInput(const int16_t *,const int &);
    [[nodiscard]] bool prevPeriodBetter(const int&,const int& ,const int&) const;
    int findPitchPeriod(const int16_t * , const int &);
    int skipPitchPeriod(const int16_t *,
                        const double &,
                        const int &);
    bool ChangeSpeed(const double &);
    bool ProcessStreamInput();
    bool AllocateStreamBuffers(const int &,const int &);
    bool enlargeInputBufferIfNeeded(const int &);
    bool AddFloatSamplesToInputBuffer(const float *,const int&);
    static void overlapAdd(const int &,
                           const int &,
                           int16_t *out,
                           const int16_t *,
                           const int16_t *);
    int insertPitchPeriod(const int16_t *,
                          const double &,
                          const int &);
    void removeInputSamples(const int &);
    bool adjustPitch(const int &);
    bool moveNewSamplesToPitchBuffer(const int &);
    static void overlapAddWithSeparation(const int &,
                                         const int & ,
                                         const int & ,
                                         int16_t *,
                                         const int16_t *,
                                         const int16_t *);
    void removePitchSamples(const int &);
    bool adjustRate(const double &,const int &);
    int16_t interpolate(const int16_t * , const int &,const int &) const;
    static int findSincCoefficient(const int &,const int &,const int &);
    static void scaleSamples(int16_t * , const int &,const double &);
    bool addShortSamplesToInputBuffer(const int16_t * , const int &);
    bool addUnsignedCharSamplesToInputBuffer(const uint8_t *,const int &);

public:
    bool Open(const int &sampleRate,const int &numChannels);
    void Close() noexcept(true);

/* Use this to write floating point data to be speed up or down into the stream.
   Values must be between -1 and 1.  Return 0 if memory realloc failed, otherwise 1 */
    virtual bool sonicWriteFloatToStream(const float *samples,const int &numSamples);
/* Use this to write 16-bit data to be speed up or down into the stream.
   Return 0 if memory realloc failed, otherwise 1 */
    virtual bool sonicWriteShortToStream(const int16_t *samples,const int &numSamples);
/* Use this to write 8-bit unsigned data to be speed up or down into the stream.
   Return 0 if memory realloc failed, otherwise 1 */
    virtual bool sonicWriteUnsignedCharToStream(const uint8_t *samples,const int &numSamples);
/* Use this to read floating point data out of the stream.  Sometimes no data
   will be available, and zero is returned, which is not an error condition. */
    virtual int sonicReadFloatFromStream(float *samples,const int &maxSamples);
/* Use this to read 16-bit data out of the stream.  Sometimes no data will
   be available, and zero is returned, which is not an error condition. */
    virtual int sonicReadShortFromStream(int16_t *samples,const int &maxSamples);
/* Use this to read 8-bit unsigned data out of the stream.  Sometimes no data will
   be available, and zero is returned, which is not an error condition. */
    virtual int sonicReadUnsignedCharFromStream(uint8_t *samples,const int &maxSamples);
/* Force the sonic stream to generate output using whatever data it currently
   has.  No extra delay will be added to the output, but flushing in the middle of
   words could introduce distortion. */
    virtual bool sonicFlushStream();
/* Return the number of samples in the output buffer */
    [[nodiscard]] virtual int sonicSamplesAvailable() const;
/* Get the speed of the stream. */
    [[nodiscard]] virtual double sonicGetSpeed() const;
/* Set the speed of the stream. */
    virtual void sonicSetSpeed(const double &);
/* Get the pitch of the stream. */
    [[nodiscard]] virtual double sonicGetPitch() const;
/* Set the pitch of the stream. */
    virtual void sonicSetPitch(const double &);
/* Get the rate of the stream. */
    [[nodiscard]] virtual double sonicGetRate() const;
/* Set the rate of the stream. */
    virtual void sonicSetRate(const double &);
/* Get the scaling factor of the stream. */
    [[nodiscard]] virtual double sonicGetVolume();
/* Set the scaling factor of the stream. */
    virtual void sonicSetVolume(const double &);
/* Get the chord pitch setting. */
    [[nodiscard]] virtual int sonicGetChordPitch() const;
/* Set chord pitch mode on or off.  Default is off.  See the documentation
   page for a description of this feature. */
    virtual void sonicSetChordPitch(const int &);
/* Get the quality setting. */
    [[nodiscard]] virtual int sonicGetQuality();
/* Set the "quality".  Default 0 is virtually as good as 1, but very much faster. */
    virtual void sonicSetQuality(const int &);
/* Get the sample rate of the stream. */
    [[nodiscard]] virtual int sonicGetSampleRate() const;
/* Set the sample rate of the stream.  This will drop any samples that have not been read. */
//    virtual void sonicSetSampleRate(int sampleRate);
/* Get the number of channels. */
    [[nodiscard]] virtual int sonicGetNumChannels() const;
/* Set the number of channels.  This will drop any samples that have not been read. */
//    virtual void sonicSetNumChannels(int numChannels);
/* This is a non-stream oriented interface to just change the speed of a sound
   sample.  It works in-place on the sample array, so there must be at least
   speed*numSamples available space in the array. Returns the new number of samples. */
    [[nodiscard]] virtual int sonicChangeFloatSpeed(float *samples,
                                                    int numSamples,
                                                    double speed,
                                                    double pitch,
                                                    double rate,
                                                    double volume,
                                                    int useChordPitch,
                                                    int sampleRate,
                                                    int numChannels);
/* This is a non-stream oriented interface to just change the speed of a sound
   sample.  It works in-place on the sample array, so there must be at least
   speed*numSamples available space in the array. Returns the new number of samples. */
    [[nodiscard]] virtual int sonicChangeShortSpeed(int16_t *samples,
                                                    int numSamples,
                                                    double speed,
                                                    double pitch,
                                                    double rate,
                                                    double volume,
                                                    int useChordPitch,
                                                    int sampleRate,
                                                    int numChannels);

protected:
    std::vector<int16_t> m_inputBuffer,m_outputBuffer,
                        m_pitchBuffer,m_downSampleBuffer;
#if 0
    short *m_inputBuffer;
    short *m_outputBuffer;
    short *m_pitchBuffer;
    short *m_downSampleBuffer;
#endif
    double m_speed{},m_volume{},m_pitch{},m_rate{},m_avePower{};
    int m_oldRatePosition{},m_newRatePosition{},m_useChordPitch{},m_quality{},
            m_numChannels{},m_inputBufferSize{},m_pitchBufferSize{},m_outputBufferSize{},
            m_numInputSamples{},m_numOutputSamples{},m_numPitchSamples{},m_minPeriod{},
            m_maxPeriod{},m_maxRequired{},m_remainingInputToCopy{},m_sampleRate{},
            m_prevPeriod{},m_prevMinDiff{};
public:
    X_DISABLE_COPY_MOVE(XSonic)
    explicit XSonic() = default;
    explicit XSonic(const int &sampleRate,const int &numChannels);
    virtual ~XSonic() = default;
};

#endif
