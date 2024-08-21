#include "XSonic.hpp"
#include <algorithm>
#include <iostream>

bool XSonic::Open(const int &sampleRate,const int &numChannels){
    return AllocateStreamBuffers(sampleRate,numChannels);
}

void XSonic::Close() noexcept(true) {
    m_inputBuffer.clear();
    m_outputBuffer.clear();
    m_pitchBuffer.clear();
    m_downSampleBuffer.clear();
}

bool XSonic::sonicWriteFloatToStream(const float *samples,const int &numSamples) {

    if (!AddFloatSamplesToInputBuffer(samples,numSamples)){
        return {};
    }

    return ProcessStreamInput();
}

bool XSonic::sonicWriteShortToStream(const int16_t *samples,const int &numSamples) {

    if (!addShortSamplesToInputBuffer(samples,numSamples)) {
        return {};
    }
    return ProcessStreamInput();
}

bool XSonic::sonicWriteUnsignedCharToStream(const uint8_t *samples,const int &numSamples) {

    if (!addUnsignedCharSamplesToInputBuffer(samples,numSamples)){
        return {};
    }

    return ProcessStreamInput();
}

int XSonic::sonicReadFloatFromStream(float *samples,const int &maxSamples) {

    if(!samples || maxSamples <= 0) {
        return {};
    }

    auto numSamples{m_numOutputSamples};

    int remainingSamples {};
    if(numSamples > maxSamples) {
        remainingSamples = numSamples - maxSamples;
        numSamples = maxSamples;
    }

    auto out_buffer{m_outputBuffer.data()};
    auto count{numSamples * m_numChannels};

    while(count--) {
        const auto v {*out_buffer++};
        *samples++ = static_cast<float>(v) / 32767.0f;
    }

    if(remainingSamples > 0) {

        auto src_{m_outputBuffer.data() + numSamples * m_numChannels};
        auto dst_{m_outputBuffer.data()};
        const auto size_{remainingSamples * m_numChannels};
        std::move(src_,src_ + size_,dst_);
    }

    m_numOutputSamples = remainingSamples;
    return numSamples;
}

int XSonic::sonicReadShortFromStream(int16_t *samples,const int &maxSamples) {

    if (!samples || maxSamples <= 0){
        return {};
    }

    auto numSamples{m_numOutputSamples};

    if(!numSamples) {
        return {};
    }

    int remainingSamples{};
    if(numSamples > maxSamples) {
        remainingSamples = numSamples - maxSamples;
        numSamples = maxSamples;
    }

    std::copy_n(m_outputBuffer.data(),numSamples * m_numChannels,samples);

    //memcpy(samples, stream->outputBuffer, numSamples*sizeof(short)*stream->numChannels);

    if(remainingSamples > 0) {
//        memmove(stream->outputBuffer, stream->outputBuffer + numSamples*stream->numChannels,
//                remainingSamples*sizeof(short)*stream->numChannels);

        auto src_{m_outputBuffer.data() + numSamples * m_numChannels};
        auto dst_{m_outputBuffer.data()};
        const auto size_{remainingSamples * m_numChannels};
        std::move(src_,src_ + size_,dst_);
    }

    m_numOutputSamples = remainingSamples;
    return numSamples;
}

int XSonic::sonicReadUnsignedCharFromStream(uint8_t *samples,const int &maxSamples) {

    if (!samples || maxSamples <= 0){
        return {};
    }

    auto numSamples{m_numOutputSamples};

    if(!numSamples) {
        return {};
    }

    int remainingSamples{};
    if(numSamples > maxSamples) {
        remainingSamples = numSamples - maxSamples;
        numSamples = maxSamples;
    }

    auto out_buffer{m_outputBuffer.data()};
    auto count{numSamples * m_numChannels};

    while(count--) {
        const auto v1 {*out_buffer++};
        const auto v2 {static_cast<char>(v1 >> 8)};
        *samples++ = v2 + 128;
    }

    if(remainingSamples > 0) {
        auto src_{m_outputBuffer.data() + numSamples * m_numChannels};
        auto dst_{m_outputBuffer.data()};
        const auto size_{remainingSamples * m_numChannels};
        std::move(src_,src_ + size_,dst_);
    }

    m_numOutputSamples = remainingSamples;

    return numSamples;
}

bool XSonic::sonicFlushStream() {

    const auto maxRequired{m_maxRequired};
    const auto remainingSamples{m_numInputSamples};
    const auto speed{m_speed / m_pitch};
    const auto rate{m_rate * m_pitch};

    const auto temp_v{(remainingSamples / speed + m_numPitchSamples) / rate + 0.5f};
    const auto expectedOutputSamples{m_numOutputSamples + static_cast<int>(temp_v)};

    /* Add enough silence to flush both input and pitch buffers. */
    if(!enlargeInputBufferIfNeeded(remainingSamples + 2 * maxRequired)){
        return {};
    }

    auto dst_{m_inputBuffer.data() + remainingSamples * m_numChannels};
    const auto size_{2 * maxRequired * m_numChannels};
    std::fill_n(dst_,size_,0);

    m_numInputSamples += 2 * maxRequired;

    if(!sonicWriteShortToStream({},{})) {
        return {};
    }

    /* Throw away any extra samples we generated due to the silence we added */
    if(m_numOutputSamples > expectedOutputSamples) {
        m_numOutputSamples = expectedOutputSamples;
    }

    /* Empty input and pitch buffers */
    m_numInputSamples = m_remainingInputToCopy = m_numPitchSamples = 0;

    return true;
}

int XSonic::sonicSamplesAvailable() const{
    return m_numOutputSamples;
}

float XSonic::sonicGetSpeed() const{
    return m_speed;
}

void XSonic::sonicSetSpeed(const float &speed) {
    m_speed = speed;
}

float XSonic::sonicGetPitch() const{
    return m_pitch;
}

void XSonic::sonicSetPitch(const float &pitch) {
    m_pitch = pitch;
}

float XSonic::sonicGetRate() const{
    return m_rate;
}

void XSonic::sonicSetRate(const float &rate) {
    m_rate = rate;
}

float XSonic::sonicGetVolume() {
    return m_volume;
}

void XSonic::sonicSetVolume(const float &volume) {
    m_volume = volume;
}

int XSonic::sonicGetChordPitch() const{
    return m_useChordPitch;
}

void XSonic::sonicSetChordPitch(const int &useChordPitch) {
    m_useChordPitch = useChordPitch;
}

int XSonic::sonicGetQuality() {
    return m_quality;
}

void XSonic::sonicSetQuality(const int &quality) {
    m_quality = quality;
}

int XSonic::sonicGetSampleRate() const{
    return m_sampleRate;
}

//void XSonic::sonicSetSampleRate(int sampleRate) {
//
//}

int XSonic::sonicGetNumChannels() const{
    return m_numChannels;
}

//void XSonic::sonicSetNumChannels(int numChannels) {
//
//}

int XSonic::sonicChangeFloatSpeed(float *samples,
                                  int numSamples,
                                  double speed,
                                  double pitch,
                                  double rate,
                                  double volume,
                                  int useChordPitch,
                                  int sampleRate,
                                  int numChannels) {

    Open(sampleRate, numChannels);
    sonicSetSpeed(speed);
    sonicSetPitch(pitch);
    sonicSetRate(rate);
    sonicSetVolume(volume);
    sonicSetChordPitch(useChordPitch);
    sonicWriteFloatToStream(samples, numSamples);
    sonicFlushStream();
    numSamples = sonicSamplesAvailable();
    sonicReadFloatFromStream(samples, numSamples);
    Close();
    return numSamples;
}

int XSonic::sonicChangeShortSpeed(int16_t *samples,
                                  int numSamples,
                                  double speed,
                                  double pitch,
                                  double rate,
                                  double volume,
                                  int useChordPitch,
                                  int sampleRate,
                                  int numChannels) {
    Open(sampleRate, numChannels);
    sonicSetSpeed(speed);
    sonicSetPitch(pitch);
    sonicSetRate(rate);
    sonicSetVolume(volume);
    sonicSetChordPitch(useChordPitch);
    sonicWriteShortToStream(samples, numSamples);
    sonicFlushStream();
    numSamples = sonicSamplesAvailable();
    sonicReadShortFromStream(samples, numSamples);
    Close();
    return numSamples;
}

XSonic::XSonic(const int &sampleRate,const int &numChannels){
    if (!Open(sampleRate,numChannels)){
        std::cerr << "XSonic::XSonic construct failed\n";
        return;
    }
}
