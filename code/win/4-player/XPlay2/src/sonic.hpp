#ifndef SONIC_HPP
#define SONIC_HPP

#include "XHelper.hpp"
#include <vector>

class XSonic {
    static inline constexpr auto SONIC_MIN_PITCH {65},
                                SONIC_MAX_PITCH {400},
                                SONIC_AMDF_FREQ {4000};

    bool AllocateStreamBuffers(const int &sampleRate,const int &numChannels);

public:

    virtual void Open(const int &sampleRate,const int &numChannels);

/* Use this to write floating point data to be speed up or down into the stream.
   Values must be between -1 and 1.  Return 0 if memory realloc failed, otherwise 1 */
    virtual int sonicWriteFloatToStream(float *samples, int numSamples);
/* Use this to write 16-bit data to be speed up or down into the stream.
   Return 0 if memory realloc failed, otherwise 1 */
    virtual int sonicWriteShortToStream(short *samples, int numSamples);
/* Use this to write 8-bit unsigned data to be speed up or down into the stream.
   Return 0 if memory realloc failed, otherwise 1 */
    virtual int sonicWriteUnsignedCharToStream(unsigned char *samples, int numSamples);
/* Use this to read floating point data out of the stream.  Sometimes no data
   will be available, and zero is returned, which is not an error condition. */
    virtual int sonicReadFloatFromStream(float *samples, int maxSamples);
/* Use this to read 16-bit data out of the stream.  Sometimes no data will
   be available, and zero is returned, which is not an error condition. */
    virtual int sonicReadShortFromStream(short *samples, int maxSamples);
/* Use this to read 8-bit unsigned data out of the stream.  Sometimes no data will
   be available, and zero is returned, which is not an error condition. */
    virtual int sonicReadUnsignedCharFromStream(unsigned char *samples, int maxSamples);
/* Force the sonic stream to generate output using whatever data it currently
   has.  No extra delay will be added to the output, but flushing in the middle of
   words could introduce distortion. */
    virtual int sonicFlushStream();
/* Return the number of samples in the output buffer */
    virtual int sonicSamplesAvailable();
/* Get the speed of the stream. */
    virtual float sonicGetSpeed();
/* Set the speed of the stream. */
    virtual void sonicSetSpeed(float speed);
/* Get the pitch of the stream. */
    virtual float sonicGetPitch();
/* Set the pitch of the stream. */
    virtual void sonicSetPitch(float pitch);
/* Get the rate of the stream. */
    virtual float sonicGetRate();
/* Set the rate of the stream. */
    virtual void sonicSetRate(float rate);
/* Get the scaling factor of the stream. */
    virtual float sonicGetVolume();
/* Set the scaling factor of the stream. */
    virtual void sonicSetVolume(float volume);
/* Get the chord pitch setting. */
    virtual int sonicGetChordPitch();
/* Set chord pitch mode on or off.  Default is off.  See the documentation
   page for a description of this feature. */
    virtual void sonicSetChordPitch(int useChordPitch);
/* Get the quality setting. */
    virtual int sonicGetQuality();
/* Set the "quality".  Default 0 is virtually as good as 1, but very much faster. */
    virtual void sonicSetQuality(int quality);
/* Get the sample rate of the stream. */
    virtual int sonicGetSampleRate();
/* Set the sample rate of the stream.  This will drop any samples that have not been read. */
    virtual void sonicSetSampleRate(int sampleRate);
/* Get the number of channels. */
    virtual int sonicGetNumChannels();
/* Set the number of channels.  This will drop any samples that have not been read. */
    virtual void sonicSetNumChannels(int numChannels);
/* This is a non-stream oriented interface to just change the speed of a sound
   sample.  It works in-place on the sample array, so there must be at least
   speed*numSamples available space in the array. Returns the new number of samples. */
    virtual int sonicChangeFloatSpeed(float *samples, int numSamples, float speed, float pitch,
                              float rate, float volume, int useChordPitch, int sampleRate, int numChannels);
/* This is a non-stream oriented interface to just change the speed of a sound
   sample.  It works in-place on the sample array, so there must be at least
   speed*numSamples available space in the array. Returns the new number of samples. */
    virtual int sonicChangeShortSpeed(short *samples, int numSamples, float speed, float pitch,
                              float rate, float volume, int useChordPitch, int sampleRate, int numChannels);
private:
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
    virtual ~XSonic();
};

#endif
