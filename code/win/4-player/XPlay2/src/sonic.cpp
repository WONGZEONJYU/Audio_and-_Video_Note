#include "sonic.hpp"

/* The number of points to use in the sinc FIR filter for resampling. */
static inline constexpr auto  SINC_FILTER_POINTS {12}, /* I am not able to hear improvement with higher N. */
SINC_TABLE_SIZE {601};

bool XSonic::processStreamInput(){

    const auto originalNumOutputSamples{m_numOutputSamples};
    const auto speed{m_speed / m_pitch};
    auto rate{m_rate};
    if (!m_useChordPitch){
        rate *= m_pitch;
    }

    if(speed > 1.00001 || speed < 0.99999) {
        changeSpeed(stream, speed);
    } else {
        if(!copyToOutput(stream, stream->inputBuffer, stream->numInputSamples)) {
            return 0;
        }
        stream->numInputSamples = 0;
    }

    if(stream->useChordPitch) {
        if(stream->pitch != 1.0f) {
            if(!adjustPitch(stream, originalNumOutputSamples)) {
                return 0;
            }
        }
    } else if(rate != 1.0f) {
        if(!adjustRate(stream, rate, originalNumOutputSamples)) {
            return 0;
        }
    }
    if(stream->volume != 1.0f) {
        /* Adjust output volume. */
        scaleSamples(stream->outputBuffer + originalNumOutputSamples*stream->numChannels,
                     (stream->numOutputSamples - originalNumOutputSamples)*stream->numChannels,
                     stream->volume);
    }
    return 1;

    return {};
}

bool XSonic::AllocateStreamBuffers(const int &sampleRate,const int &numChannels) {

    if (sampleRate <= 0 || numChannels <= 0){
        return {};
    }

    Close();
#if 1
    const auto minPeriod {sampleRate / SONIC_MIN_PITCH};
    const auto maxPeriod {sampleRate / SONIC_MAX_PITCH};
#else
    const auto minPeriod {sampleRate / SONIC_MAX_PITCH};
    const auto maxPeriod {sampleRate / SONIC_MIN_PITCH};
#endif
    const auto maxRequired{2 * maxPeriod};

    const auto alloc_size {maxRequired * numChannels};

    m_inputBufferSize = m_outputBufferSize = m_pitchBufferSize = maxRequired;
    m_inputBuffer.resize(alloc_size);
    m_outputBuffer.resize(alloc_size);
    m_pitchBuffer.resize(alloc_size);
    m_downSampleBuffer.resize(maxRequired);

    m_sampleRate = sampleRate;
    m_numChannels = numChannels;
    m_prevPeriod = 0;
    m_minPeriod = minPeriod;
    m_maxPeriod = maxPeriod;
    m_maxRequired = maxRequired;

    m_speed = m_pitch = m_volume = m_rate = 1.0;
    m_oldRatePosition = m_newRatePosition = m_useChordPitch = m_quality = 0;
    m_avePower = 50.0;
    return true;
}

void XSonic::enlargeInputBufferIfNeeded(const int &numSamples) {

    if (m_numInputSamples + numSamples > m_inputBufferSize){
        m_inputBufferSize += (m_inputBufferSize >> 1) + numSamples;
        m_inputBuffer.resize(m_inputBufferSize * m_numChannels);
    }
}

bool XSonic::AddFloatSamplesToInputBuffer(const float *samples, const int &numSamples) {

    if (!samples || numSamples < 0){
        return {};
    }

    if (!numSamples){
        return true;
    }

    /**
     * 计算出需拷贝的sample个数
     */
    auto count {numSamples * m_numChannels};

    /**
     * 扩大输入缓冲区
     */
    enlargeInputBufferIfNeeded(numSamples);

    auto buffer {m_inputBuffer.data() + m_numInputSamples * m_numChannels};

    while (count--){
        const auto v {static_cast<int16_t>((*samples++) * 32767.0f)};
        *buffer++ = v;
    }

    m_numInputSamples += numSamples;

    return true;
}

void XSonic::Open(const int &sampleRate,const int &numChannels){
    AllocateStreamBuffers(sampleRate,numChannels);
}

void XSonic::Close() noexcept(true) {
    m_inputBuffer.clear();
    m_outputBuffer.clear();
    m_pitchBuffer.clear();
    m_downSampleBuffer.clear();
}

bool XSonic::sonicWriteFloatToStream(float *samples, int numSamples) {

    if (!AddFloatSamplesToInputBuffer(samples,numSamples)){
        return {};
    }

    return true;
}

int XSonic::sonicWriteShortToStream(short *samples, int numSamples) {
    return 0;
}

int XSonic::sonicWriteUnsignedCharToStream(unsigned char *samples, int numSamples) {
    return 0;
}

int XSonic::sonicReadFloatFromStream(float *samples, int maxSamples) {
    return 0;
}

int XSonic::sonicReadShortFromStream(short *samples, int maxSamples) {
    return 0;
}

int XSonic::sonicReadUnsignedCharFromStream(unsigned char *samples, int maxSamples) {
    return 0;
}

int XSonic::sonicFlushStream() {
    return 0;
}

int XSonic::sonicSamplesAvailable() {
    return 0;
}

float XSonic::sonicGetSpeed() {
    return 0;
}

void XSonic::sonicSetSpeed(float speed) {

}

float XSonic::sonicGetPitch() {
    return 0;
}

void XSonic::sonicSetPitch(float pitch) {

}

float XSonic::sonicGetRate() {
    return 0;
}

void XSonic::sonicSetRate(float rate) {

}

float XSonic::sonicGetVolume() {
    return 0;
}

void XSonic::sonicSetVolume(float volume) {

}

int XSonic::sonicGetChordPitch() {
    return 0;
}

void XSonic::sonicSetChordPitch(int useChordPitch) {

}

int XSonic::sonicGetQuality() {
    return 0;
}

void XSonic::sonicSetQuality(int quality) {

}

int XSonic::sonicGetSampleRate() {
    return 0;
}

//void XSonic::sonicSetSampleRate(int sampleRate) {
//
//}

int XSonic::sonicGetNumChannels() {
    return 0;
}

//void XSonic::sonicSetNumChannels(int numChannels) {
//
//}

int XSonic::sonicChangeFloatSpeed(float *samples, int numSamples, float speed, float pitch, float rate, float volume,
                                  int useChordPitch, int sampleRate, int numChannels) {
    return 0;
}

int XSonic::sonicChangeShortSpeed(short *samples, int numSamples, float speed, float pitch, float rate, float volume,
                                  int useChordPitch, int sampleRate, int numChannels) {
    return 0;
}

XSonic::XSonic(const int &sampleRate,const int &numChannels){
    Open(sampleRate,numChannels);
}

XSonic::~XSonic() {

}


