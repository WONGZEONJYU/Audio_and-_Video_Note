#include "xsonic.hpp"
#include <algorithm>
#include <iostream>

XSonic::XSonic(const int &sampleRate,const int &numChannels) {
    if (!Open(sampleRate, numChannels)){
        std::cerr << "XSonic Construct failed\n";
    }
}

XSonic::XSonic(XSonic &&obj) noexcept(true){
    Move_(std::addressof(obj));
}

XSonic &XSonic::operator=(XSonic &&obj) noexcept(true){
    if (const auto src_{std::addressof(obj)}; this != src_){
        Move_(src_);
    }
    return *this;
}

bool XSonic::Open(const int &sampleRate,const int &numChannels) {
    const auto ret {sonicCreateStream( sampleRate, numChannels)};
    m_is_init_ = ret;
    return ret;
}

void XSonic::Close(){
    m_inputBuffer.clear();
    m_outputBuffer.clear();
    m_pitchBuffer.clear();
    m_downSampleBuffer.clear();
    const auto parent_{static_cast<XSonic_data*>(this)};
    *parent_ = {};
}

float XSonic::sonicGetSpeed() const{
    return m_speed;
}

void XSonic::sonicSetSpeed(const float &speed){
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
    m_oldRatePosition = m_newRatePosition = 0;
}

int XSonic::sonicGetChordPitch() const{
    return m_useChordPitch;
}

void XSonic::sonicSetChordPitch(const int &useChordPitch){
    m_useChordPitch = useChordPitch;
}

int XSonic::sonicGetQuality() const {
    return m_quality;
}

void XSonic::sonicSetQuality(const int &quality) {
    m_quality = quality;
}

float XSonic::sonicGetVolume() const{
    return m_volume;
}

void XSonic::sonicSetVolume(const float &volume) {
    m_volume = volume;
}

int XSonic::sonicGetSampleRate() const {
    return m_sampleRate;
}

int XSonic::sonicGetNumChannels() const {
    return m_numChannels;
}

int XSonic::sonicReadDoubleFromStream(double *samples, const int &maxSamples) {

    if (!m_is_init_ || !samples || maxSamples <= 0) {
        return -1;
    }

    auto numSamples {m_numOutputSamples};
    if(!numSamples) {
        return {};
    }

    int remainingSamples {};
    if(numSamples > maxSamples) {
        remainingSamples = numSamples - maxSamples;
        numSamples = maxSamples;
    }

    const auto out_buffer{m_outputBuffer.data()};
    const auto count{numSamples * m_numChannels};

    for(int i{};i < count;++i) {
        samples[i] = out_buffer[i] / 32768.0;
    }

    if(remainingSamples > 0) {
        const auto src_{m_outputBuffer.data() + numSamples * m_numChannels};
        const auto dst_{m_outputBuffer.data()};
        const auto size_{remainingSamples * m_numChannels};
        std::move(src_,src_ + size_,dst_);
    }

    m_numOutputSamples = remainingSamples;
    return numSamples;
}

int XSonic::sonicReadS64FromStream(int64_t *samples, const int &maxSamples) {

    if (!m_is_init_ || !samples || maxSamples <= 0) {
        return -1;
    }

    auto numSamples {m_numOutputSamples};
    if(!numSamples) {
        return {};
    }

    int remainingSamples {};
    if(numSamples > maxSamples) {
        remainingSamples = numSamples - maxSamples;
        numSamples = maxSamples;
    }

    const auto out_buffer{m_outputBuffer.data()};
    const auto count{numSamples * m_numChannels};

    for(int i{};i < count;++i) {
        samples[i] = static_cast<int64_t>(out_buffer[i]) << 48;
    }

    if(remainingSamples > 0) {
        const auto src_{m_outputBuffer.data() + numSamples * m_numChannels};
        const auto dst_{m_outputBuffer.data()};
        const auto size_{remainingSamples * m_numChannels};
        std::move(src_,src_ + size_,dst_);
    }

    m_numOutputSamples = remainingSamples;
    return numSamples;
}

int XSonic::sonicReadU64FromStream(uint64_t *samples, const int &maxSamples) {

    if (!m_is_init_ || !samples || maxSamples <= 0) {
        return -1;
    }

    auto numSamples {m_numOutputSamples};
    if(!numSamples) {
        return {};
    }

    int remainingSamples {};
    if(numSamples > maxSamples) {
        remainingSamples = numSamples - maxSamples;
        numSamples = maxSamples;
    }

    const auto out_buffer{m_outputBuffer.data()};
    const auto count{numSamples * m_numChannels};

    for(int i{};i < count;++i) {
        samples[i] = static_cast<uint64_t>(out_buffer[i] + 32768) << 48;
    }

    if(remainingSamples > 0) {
        const auto src_{m_outputBuffer.data() + numSamples * m_numChannels};
        const auto dst_{m_outputBuffer.data()};
        const auto size_{remainingSamples * m_numChannels};
        std::move(src_,src_ + size_,dst_);
    }

    m_numOutputSamples = remainingSamples;
    return numSamples;
}

int XSonic::sonicReadFloatFromStream(float *samples,
                                     const int &maxSamples) {
    if (!m_is_init_ || !samples || maxSamples <= 0) {
        return -1;
    }

    auto numSamples {m_numOutputSamples};
    if(!numSamples) {
        return {};
    }

    int remainingSamples {};
    if(numSamples > maxSamples) {
        remainingSamples = numSamples - maxSamples;
        numSamples = maxSamples;
    }

    auto out_buffer{m_outputBuffer.data()};
    auto count{numSamples * m_numChannels};

    while(count--) {
#if 0
        *samples++ = (*out_buffer++)/32767.0f;
#else
        const auto v{*out_buffer++};
        *samples++ = static_cast<float>(v) / 32767.0f;
#endif
    }

    if(remainingSamples > 0) {
        const auto src_{m_outputBuffer.data() + numSamples * m_numChannels};
        const auto dst_{m_outputBuffer.data()};
        const auto size_{remainingSamples * m_numChannels};
        std::move(src_,src_ + size_,dst_);
    }

    m_numOutputSamples = remainingSamples;
    return numSamples;
}

int XSonic::sonicReadU32FromStream(uint32_t *samples, const int &maxSamples) {

    if (!m_is_init_ || !samples || maxSamples <= 0) {
        return -1;
    }

    auto numSamples {m_numOutputSamples};
    if(!numSamples) {
        return {};
    }

    int remainingSamples {};
    if(numSamples > maxSamples) {
        remainingSamples = numSamples - maxSamples;
        numSamples = maxSamples;
    }

    const auto out_buffer{m_outputBuffer.data()};
    const auto count{numSamples * m_numChannels};

    for(int i{};i < count;++i) {
        samples[i] = static_cast<uint32_t>(out_buffer[i] + 32768) << 16;
    }

    if(remainingSamples > 0) {
        const auto src_{m_outputBuffer.data() + numSamples * m_numChannels};
        const auto dst_{m_outputBuffer.data()};
        const auto size_{remainingSamples * m_numChannels};
        std::move(src_,src_ + size_,dst_);
    }

    m_numOutputSamples = remainingSamples;
    return numSamples;
}

int XSonic::sonicReadS32FromStream(int32_t *samples, const int &maxSamples) {
    if (!m_is_init_ || !samples || maxSamples <= 0) {
        return -1;
    }

    auto numSamples {m_numOutputSamples};
    if(!numSamples) {
        return {};
    }

    int remainingSamples {};
    if(numSamples > maxSamples) {
        remainingSamples = numSamples - maxSamples;
        numSamples = maxSamples;
    }

    const auto out_buffer{m_outputBuffer.data()};
    const auto count{numSamples * m_numChannels};


    for(int i{};i < count;++i) {
        samples[i] = out_buffer[i] << 16;
    }

    if(remainingSamples > 0) {
        const auto src_{m_outputBuffer.data() + numSamples * m_numChannels};
        const auto dst_{m_outputBuffer.data()};
        const auto size_{remainingSamples * m_numChannels};
        std::move(src_,src_ + size_,dst_);
    }

    m_numOutputSamples = remainingSamples;
    return numSamples;
}

int XSonic::sonicReadShortFromStream(int16_t *samples,
                                     const int &maxSamples) {

    if (!m_is_init_ || !samples || maxSamples <= 0){
        return -1;
    }

    auto numSamples{m_numOutputSamples};

    if(!numSamples) {
        return {};
    }

    int remainingSamples {};
    if(numSamples > maxSamples) {
        remainingSamples = numSamples - maxSamples;
        numSamples = maxSamples;
    }

    const auto read_{m_outputBuffer.data()};
    const auto write_{samples};
    const auto w_size_{numSamples * m_numChannels};
    std::copy_n(read_,w_size_,write_);

    if(remainingSamples > 0) {
        const auto src_{m_outputBuffer.data() + numSamples*m_numChannels};
        const auto dst_{m_outputBuffer.data()};
        const auto size_{remainingSamples * m_numChannels};
        std::move(src_,src_ + size_,dst_);
    }

    m_numOutputSamples = remainingSamples;
    return numSamples;
}

int XSonic::sonicReadUnsignedShortFromStream(uint16_t *samples, const int &maxSamples) {

    if (!m_is_init_ || !samples || maxSamples <= 0){
        return -1;
    }

    auto numSamples{m_numOutputSamples};

    if(!numSamples) {
        return {};
    }

    int remainingSamples {};
    if(numSamples > maxSamples) {
        remainingSamples = numSamples - maxSamples;
        numSamples = maxSamples;
    }

    const auto read_{m_outputBuffer.data()};
    const auto w_size_{numSamples * m_numChannels};

    for(int i{}; i < w_size_;++i) {
        samples[i] = read_[i] + 32768;
    }

    if(remainingSamples > 0) {
        const auto src_{m_outputBuffer.data() + numSamples * m_numChannels};
        const auto dst_{m_outputBuffer.data()};
        const auto size_{remainingSamples * m_numChannels};
        std::move(src_,src_ + size_,dst_);
    }

    m_numOutputSamples = remainingSamples;
    return numSamples;
}

int XSonic::sonicReadUnsignedCharFromStream(uint8_t *samples,
                                            const int &maxSamples) {

    if (!m_is_init_ || !samples || maxSamples <= 0){
        return -1;
    }

    auto numSamples{m_numOutputSamples};

    if(!numSamples) {
        return 0;
    }

    int remainingSamples{};
    if(numSamples > maxSamples) {
        remainingSamples = numSamples - maxSamples;
        numSamples = maxSamples;
    }

    const auto out_buffer{m_outputBuffer.data()};
    const auto count{numSamples * m_numChannels};

    for(int i{};i < count;++i) {
        const auto v1{out_buffer[i] >> 8},v2{v1 + 128};
        samples[i] = static_cast<uint8_t>(v2);
    }

    if(remainingSamples > 0) {
        const auto src_{m_outputBuffer.data() + numSamples * m_numChannels};
        const auto dst_{m_outputBuffer.data()};
        const auto size_{remainingSamples*m_numChannels};
        std::move(src_,src_ + size_,dst_);
    }
    m_numOutputSamples = remainingSamples;
    return numSamples;
}

int XSonic::sonicReadSignedCharFromStream(int8_t *samples, const int &maxSamples) {

    if (!m_is_init_ || !samples || maxSamples <= 0){
        return -1;
    }

    auto numSamples{m_numOutputSamples};

    if(!numSamples) {
        return 0;
    }

    int remainingSamples{};
    if(numSamples > maxSamples) {
        remainingSamples = numSamples - maxSamples;
        numSamples = maxSamples;
    }

    const auto out_buffer{m_outputBuffer.data()};
    const auto count{numSamples * m_numChannels};

    for(int i{}; i < count; ++i) {
        samples[i] = static_cast<int8_t>(std::round(out_buffer[i] >> 8));
    }

    if(remainingSamples > 0) {
        const auto src_{m_outputBuffer.data() + numSamples * m_numChannels};
        const auto dst_{m_outputBuffer.data()};
        const auto size_{remainingSamples*m_numChannels};
        std::move(src_,src_ + size_,dst_);
    }

    m_numOutputSamples = remainingSamples;
    return numSamples;
}

bool XSonic::sonicFlushStream() {

    if (!m_is_init_){
        return {};
    }

    const auto maxRequired{m_maxRequired},
                remainingSamples{m_numInputSamples};

    const auto speed{m_speed / m_pitch},
                rate{m_rate * m_pitch};

    const auto f_remainingSamples{static_cast<float >(remainingSamples)},
                f_m_numPitchSamples{static_cast<float >(m_numPitchSamples)},
                temp_{(f_remainingSamples / speed + f_m_numPitchSamples) / rate + 0.5f};

    const auto expectedOutputSamples{m_numOutputSamples + static_cast<int>(temp_)};

    /* Add enough silence to flush both input and pitch buffers. */
    if(!enlargeInputBufferIfNeeded(remainingSamples + 2 * maxRequired)) {
        return {};
    }

    const auto dst_{m_inputBuffer.data() + remainingSamples * m_numChannels};
    const auto size_{2 * maxRequired * m_numChannels};
    std::fill_n(dst_,size_,0);

    m_numInputSamples += 2 * maxRequired;

//    if(!sonicWriteShortToStream({},{})) { //这个逻辑感觉有点多余,直接调用processStreamInput进行处理
//        return {};
//    }

    if (!processStreamInput()){
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

bool XSonic::sonicWriteDoubleToStream(const double *samples, const int &numSamples) {
    if (!m_is_init_ || !samples || numSamples <= 0){
        return {};
    }

    if(!addDoubleSamplesToInputBuffer(samples, numSamples)) {
        return {};
    }

    return processStreamInput();
}

bool XSonic::sonicWriteFloatToStream(const float *samples,const int &numSamples) {

    if (!m_is_init_ || !samples || numSamples <= 0){
        return {};
    }

    if(!addFloatSamplesToInputBuffer(samples, numSamples)) {
        return {};
    }

    return processStreamInput();
}

bool XSonic::sonicWriteU64ToStream(const uint64_t *samples, const int &numSamples) {

    if (!m_is_init_ || !samples || numSamples <= 0){
        return {};
    }

    if(!addU64SamplesToInputBuffer(samples, numSamples)) {
        return {};
    }

    return processStreamInput();
}

bool XSonic::sonicWriteS64ToStream(const int64_t *samples, const int &numSamples) {
    if (!m_is_init_ || !samples || numSamples <= 0){
        return {};
    }

    if(!addS64SamplesToInputBuffer(samples, numSamples)) {
        return {};
    }

    return processStreamInput();
}

bool XSonic::sonicWriteS32ToStream(const int32_t *samples, const int &numSamples) {

    if (!m_is_init_ || !samples || numSamples <= 0){
        return {};
    }

    if (!addS32SamplesToInputBuffer(samples, numSamples)) {
        return {};
    }

    return processStreamInput();
}

bool XSonic::sonicWriteU32ToStream(const uint32_t *samples, const int &numSamples) {

    if (!m_is_init_ || !samples || numSamples <= 0){
        return {};
    }

    if (!addU32SamplesToInputBuffer(samples, numSamples)) {
        return {};
    }

    return processStreamInput();
}

bool XSonic::sonicWriteShortToStream(const int16_t *samples,const int &numSamples) {

    if (!m_is_init_ || !samples || numSamples <= 0) {
        return {};
    }

    if(!addShortSamplesToInputBuffer( samples, numSamples)) {
        return {};
    }

    return processStreamInput();
}

bool XSonic::sonicWriteUnsignedShortToStream(const uint16_t *samples, const int &numSamples) {
    if (!m_is_init_ || !samples || numSamples <= 0) {
        return {};
    }

    if(!addUnsignedShortSamplesToInputBuffer(samples, numSamples)) {
        return {};
    }

    return processStreamInput();
}

bool XSonic::sonicWriteCharToStream(const int8_t *samples, const int &numSamples) {
    if (!m_is_init_ || !samples || numSamples <= 0) {
        return {};
    }

    if(!addCharSamplesToInputBuffer(samples, numSamples)) {
        return {};
    }

    return processStreamInput();
}

bool XSonic::sonicWriteUnsignedCharToStream(const uint8_t *samples,const int &numSamples) {

    if (!m_is_init_ || !samples || numSamples <= 0) {
        return {};
    }

    if(!addUnsignedCharSamplesToInputBuffer(samples, numSamples)) {
        return {};
    }
    return processStreamInput();
}

int sonicChangeFloatSpeed(float *samples,
                          const int &numSamples,
                          const float &speed,
                          const float &pitch,
                          const float &rate,
                          const float &volume,
                          const int &useChordPitch,
                          const int &sampleRate,
                          const int &numChannels) {

    if (!samples || numSamples <= 0){
        return -1;
    }

    XSonic xSonic;
    if (!xSonic.Open(sampleRate,numChannels)){
        return -1;
    }
    xSonic.sonicSetSpeed(speed);
    xSonic.sonicSetPitch(pitch);
    xSonic.sonicSetRate(rate);
    xSonic.sonicSetVolume(volume);
    xSonic.sonicSetChordPitch(useChordPitch);
    xSonic.sonicWriteFloatToStream(samples,numSamples);
    xSonic.sonicFlushStream();
    const auto ret{xSonic.sonicSamplesAvailable()};
    xSonic.sonicReadFloatFromStream(samples,ret);
    return ret;
}

int sonicChangeShortSpeed(int16_t *samples,
                          const int &numSamples,
                          const float &speed,
                          const float &pitch,
                          const float &rate,
                          const float &volume,
                          const int &useChordPitch,
                          const int &sampleRate,
                          const int &numChannels) {
    XSonic xSonic;
    if (xSonic.Open(sampleRate,numChannels)){
        return -1;
    }
    xSonic.sonicSetSpeed(speed);
    xSonic.sonicSetPitch(pitch);
    xSonic.sonicSetRate(rate);
    xSonic.sonicSetVolume(volume);
    xSonic.sonicSetChordPitch(useChordPitch);
    xSonic.sonicWriteShortToStream(samples,numSamples);
    xSonic.sonicFlushStream();
    const auto ret{xSonic.sonicSamplesAvailable()};
    xSonic.sonicReadShortFromStream(samples,ret);
    return ret;
}
