#include "xsonic.hpp"

static inline constexpr auto SINC_FILTER_POINTS{12},
SINC_TABLE_SIZE{601};

static inline constexpr int16_t sincTable[SINC_TABLE_SIZE] = {
        0, 0, 0, 0, 0, 0, 0, -1, -1, -2, -2, -3, -4, -6, -7, -9, -10, -12, -14,
        -17, -19, -21, -24, -26, -29, -32, -34, -37, -40, -42, -44, -47, -48, -50,
        -51, -52, -53, -53, -53, -52, -50, -48, -46, -43, -39, -34, -29, -22, -16,
        -8, 0, 9, 19, 29, 41, 53, 65, 79, 92, 107, 121, 137, 152, 168, 184, 200,
        215, 231, 247, 262, 276, 291, 304, 317, 328, 339, 348, 357, 363, 369, 372,
        374, 375, 373, 369, 363, 355, 345, 332, 318, 300, 281, 259, 234, 208, 178,
        147, 113, 77, 39, 0, -41, -85, -130, -177, -225, -274, -324, -375, -426,
        -478, -530, -581, -632, -682, -731, -779, -825, -870, -912, -951, -989,
        -1023, -1053, -1080, -1104, -1123, -1138, -1149, -1154, -1155, -1151,
        -1141, -1125, -1105, -1078, -1046, -1007, -963, -913, -857, -796, -728,
        -655, -576, -492, -403, -309, -210, -107, 0, 111, 225, 342, 462, 584, 708,
        833, 958, 1084, 1209, 1333, 1455, 1575, 1693, 1807, 1916, 2022, 2122, 2216,
        2304, 2384, 2457, 2522, 2579, 2625, 2663, 2689, 2706, 2711, 2705, 2687,
        2657, 2614, 2559, 2491, 2411, 2317, 2211, 2092, 1960, 1815, 1658, 1489,
        1308, 1115, 912, 698, 474, 241, 0, -249, -506, -769, -1037, -1310, -1586,
        -1864, -2144, -2424, -2703, -2980, -3254, -3523, -3787, -4043, -4291,
        -4529, -4757, -4972, -5174, -5360, -5531, -5685, -5819, -5935, -6029,
        -6101, -6150, -6175, -6175, -6149, -6096, -6015, -5905, -5767, -5599,
        -5401, -5172, -4912, -4621, -4298, -3944, -3558, -3141, -2693, -2214,
        -1705, -1166, -597, 0, 625, 1277, 1955, 2658, 3386, 4135, 4906, 5697, 6506,
        7332, 8173, 9027, 9893, 10769, 11654, 12544, 13439, 14335, 15232, 16128,
        17019, 17904, 18782, 19649, 20504, 21345, 22170, 22977, 23763, 24527,
        25268, 25982, 26669, 27327, 27953, 28547, 29107, 29632, 30119, 30569,
        30979, 31349, 31678, 31964, 32208, 32408, 32565, 32677, 32744, 32767,
        32744, 32677, 32565, 32408, 32208, 31964, 31678, 31349, 30979, 30569,
        30119, 29632, 29107, 28547, 27953, 27327, 26669, 25982, 25268, 24527,
        23763, 22977, 22170, 21345, 20504, 19649, 18782, 17904, 17019, 16128,
        15232, 14335, 13439, 12544, 11654, 10769, 9893, 9027, 8173, 7332, 6506,
        5697, 4906, 4135, 3386, 2658, 1955, 1277, 625, 0, -597, -1166, -1705,
        -2214, -2693, -3141, -3558, -3944, -4298, -4621, -4912, -5172, -5401,
        -5599, -5767, -5905, -6015, -6096, -6149, -6175, -6175, -6150, -6101,
        -6029, -5935, -5819, -5685, -5531, -5360, -5174, -4972, -4757, -4529,
        -4291, -4043, -3787, -3523, -3254, -2980, -2703, -2424, -2144, -1864,
        -1586, -1310, -1037, -769, -506, -249, 0, 241, 474, 698, 912, 1115, 1308,
        1489, 1658, 1815, 1960, 2092, 2211, 2317, 2411, 2491, 2559, 2614, 2657,
        2687, 2705, 2711, 2706, 2689, 2663, 2625, 2579, 2522, 2457, 2384, 2304,
        2216, 2122, 2022, 1916, 1807, 1693, 1575, 1455, 1333, 1209, 1084, 958, 833,
        708, 584, 462, 342, 225, 111, 0, -107, -210, -309, -403, -492, -576, -655,
        -728, -796, -857, -913, -963, -1007, -1046, -1078, -1105, -1125, -1141,
        -1151, -1155, -1154, -1149, -1138, -1123, -1104, -1080, -1053, -1023, -989,
        -951, -912, -870, -825, -779, -731, -682, -632, -581, -530, -478, -426,
        -375, -324, -274, -225, -177, -130, -85, -41, 0, 39, 77, 113, 147, 178,
        208, 234, 259, 281, 300, 318, 332, 345, 355, 363, 369, 373, 375, 374, 372,
        369, 363, 357, 348, 339, 328, 317, 304, 291, 276, 262, 247, 231, 215, 200,
        184, 168, 152, 137, 121, 107, 92, 79, 65, 53, 41, 29, 19, 9, 0, -8, -16,
        -22, -29, -34, -39, -43, -46, -48, -50, -52, -53, -53, -53, -52, -51, -50,
        -48, -47, -44, -42, -40, -37, -34, -32, -29, -26, -24, -21, -19, -17, -14,
        -12, -10, -9, -7, -6, -4, -3, -2, -2, -1, -1, 0, 0, 0, 0, 0, 0, 0
};

void XSonic::scaleSamples(int16_t *samples,
                          const int &numSamples,
                          const float & volume) {

    const auto fixedPointVolume{static_cast<int>(volume * 4096.0f)};
    auto numSamples_{numSamples};

    while(numSamples_--) {
        const auto sample{*samples};
        auto value{(sample * fixedPointVolume) >> 12};
        if(value > 32767) {
            value = 32767;
        }else if(value < -32767) {
            value = -32767;
        }else{}
        *samples++ = static_cast<int16_t>(value);
    }
}

bool XSonic::allocateStreamBuffers(const int &sampleRate,const int &numChannels) {

    if (sampleRate <= 0 || numChannels <= 0){
        return {};
    }

    Close();
    const auto minPeriod{sampleRate / SONIC_MAX_PITCH_},
                maxPeriod{sampleRate/SONIC_MIN_PITCH_},
                maxRequired{2 * maxPeriod};

    m_inputBufferSize = m_outputBufferSize = m_pitchBufferSize = maxRequired;
    try {
        const auto alloc_size{maxRequired * numChannels};
        CHECK_EXC(m_inputBuffer.resize(alloc_size));
        CHECK_EXC(m_outputBuffer.resize(alloc_size));
        CHECK_EXC(m_pitchBuffer.resize(alloc_size));
        CHECK_EXC(m_downSampleBuffer.resize(maxRequired));
    } catch (const std::exception &e) {
        Close();
        std::cerr << e.what() << "\n";
        return {};
    }

    m_sampleRate = sampleRate;
    m_numChannels = numChannels;
    m_minPeriod = minPeriod;
    m_maxPeriod = maxPeriod;
    m_maxRequired = maxRequired;
    m_speed = m_pitch = m_volume = m_rate = 1.0f;
    m_oldRatePosition = m_newRatePosition = m_useChordPitch = m_quality = m_prevPeriod = 0;
    m_avePower = 50.0f;
    return true;
}

bool XSonic::sonicCreateStream(const int &sampleRate,const int &numChannels){

    if (!allocateStreamBuffers(sampleRate,numChannels)){
        return {};
    }
    return true;
}

bool XSonic::enlargeOutputBufferIfNeeded(const int &numSamples) {

    if(m_numOutputSamples + numSamples > m_outputBufferSize) {
        m_outputBufferSize += (m_outputBufferSize >> 1) + numSamples;
        m_outputBuffer.clear();
        TRY_CATCH(CHECK_EXC(m_outputBuffer.resize(m_outputBufferSize * m_numChannels)),return {});
    }
    return true;
}

bool XSonic::enlargeInputBufferIfNeeded(const int &numSamples) {

    if(m_numInputSamples + numSamples > m_inputBufferSize) {
        m_inputBufferSize += (m_inputBufferSize >> 1) + numSamples;
        m_inputBuffer.clear();
        TRY_CATCH(CHECK_EXC(m_inputBuffer.resize(m_inputBufferSize * m_numChannels)),return {});
    }
    return true;
}

bool XSonic::addDoubleSamplesToInputBuffer(const double *samples, const int &numSamples) {

    if(numSamples <= 0 || !samples) {
        return {};
    }

    if(!enlargeInputBufferIfNeeded(numSamples)) {
        return {};
    }

    const auto count{numSamples * m_numChannels};
    const auto buffer{m_inputBuffer.data() + m_numInputSamples * m_numChannels};

    for(int i{}; i < count; ++i) {
        auto sample{samples[i]};
        if (sample > 1.0) {
            sample = 1.0;
        }

        if (sample < -1.0) {
            sample = -1.0;
        }

        buffer[i] = static_cast<int16_t>(std::round(sample * 32767.0));
    }

    m_numInputSamples += numSamples;
    return true;
}

bool XSonic::addS64SamplesToInputBuffer(const int64_t *samples, const int &numSamples) {

    if(numSamples <= 0 || !samples) {
        return {};
    }

    if(!enlargeInputBufferIfNeeded(numSamples)) {
        return {};
    }

    const auto count{numSamples * m_numChannels};
    const auto buffer{m_inputBuffer.data() + m_numInputSamples * m_numChannels};

    for(int i{}; i < count; ++i) {
        const auto v1{samples[i] + (1LL << 47)},
                v2{v1 >> 48};
        buffer[i] = static_cast<int16_t>(v2);
    }

    m_numInputSamples += numSamples;
    return true;
}

bool XSonic::addU64SamplesToInputBuffer(const uint64_t *samples, const int &numSamples) {

    if(numSamples <= 0 || !samples) {
        return {};
    }

    if(!enlargeInputBufferIfNeeded(numSamples)) {
        return {};
    }

    const auto count{numSamples * m_numChannels};
    const auto buffer{m_inputBuffer.data() + m_numInputSamples * m_numChannels};

    for(int i{}; i < count; ++i) {
        const auto v1{samples[i] - 9223372036854775808ULL},
                v2{v1 >> 48};
        buffer[i] = static_cast<int16_t>(v2);
    }

    m_numInputSamples += numSamples;
    return true;
}

bool XSonic::addFloatSamplesToInputBuffer(const float *samples,const int &numSamples) {

    if(numSamples <= 0 || !samples) {
        return {};
    }

    if(!enlargeInputBufferIfNeeded(numSamples)) {
        return {};
    }

    const auto count{numSamples * m_numChannels};
    const auto buffer{m_inputBuffer.data() + m_numInputSamples * m_numChannels};

    for(int i{}; i < count; ++i) {
        const auto v{samples[i] * 32767.0f};
        buffer[i] = static_cast<int16_t>(v);
    }

    m_numInputSamples += numSamples;
    return true;
}

bool XSonic::addS32SamplesToInputBuffer(const int32_t *samples, const int &numSamples) {

    if(numSamples <= 0 || !samples) {
        return {};
    }

    if(!enlargeInputBufferIfNeeded(numSamples)) {
        return {};
    }

    const auto count{numSamples * m_numChannels};
    const auto buffer{m_inputBuffer.data() + m_numInputSamples * m_numChannels};
    for(int i{}; i < count; ++i) {
        const auto v1{samples[i] + 32768},
                    v2{v1 >> 16};
        buffer[i] = static_cast<int16_t>(v2);
    }

    m_numInputSamples += numSamples;
    return true;
}

bool XSonic::addU32SamplesToInputBuffer(const uint32_t *samples, const int &numSamples) {

    if(numSamples <= 0 || !samples) {
        return {};
    }

    if(!enlargeInputBufferIfNeeded(numSamples)) {
        return {};
    }

    const auto count{numSamples * m_numChannels};
    const auto buffer{m_inputBuffer.data() + m_numInputSamples * m_numChannels};
    for(int i{}; i < count; ++i) {
        const auto v1{samples[i] - 2147483648L},
                v2{v1 >> 16};
        buffer[i] = static_cast<int16_t>(v2);
    }

    m_numInputSamples += numSamples;
    return true;
}

bool XSonic::addShortSamplesToInputBuffer(const int16_t *samples,const int &numSamples) {

    if(numSamples <= 0 || !samples) {
        return {};
    }

    if(!enlargeInputBufferIfNeeded(numSamples)) {
        return {};
    }

    const auto src_{samples};
    const auto dst_{m_inputBuffer.data() + m_numInputSamples * m_numChannels};
    const auto size_{numSamples * m_numChannels};
    std::copy_n(src_,size_,dst_);

    m_numInputSamples += numSamples;
    return true;
}

bool XSonic::addUnsignedShortSamplesToInputBuffer(const uint16_t *samples, const int &numSamples) {

    if(numSamples <= 0 || !samples) {
        return {};
    }

    if(!enlargeInputBufferIfNeeded(numSamples)) {
        return {};
    }

    const auto dst_{m_inputBuffer.data() + m_numInputSamples * m_numChannels};
    const auto size_{numSamples * m_numChannels};

    for(int i{}; i < size_; ++i) {
        dst_[i] = static_cast<int16_t>(samples[i] - 32768);
    }

    m_numInputSamples += numSamples;
    return true;
}

bool XSonic::addUnsignedCharSamplesToInputBuffer(const uint8_t *samples,const int &numSamples) {

    if(numSamples <= 0 || !samples) {
        return {};
    }

    if(!enlargeInputBufferIfNeeded(numSamples)) {
        return {};
    }

    const auto count{numSamples * m_numChannels};
    const auto buffer{m_inputBuffer.data() + m_numInputSamples * m_numChannels};

    for (int i{};i < count;++i){
        const auto v1{samples[i] - 128},
                v2{v1 << 8};
        buffer[i] = static_cast<int16_t>(v2);
    }

    m_numInputSamples += numSamples;
    return true;
}

bool XSonic::addCharSamplesToInputBuffer(const int8_t *samples, const int &numSamples) {

    if(numSamples <= 0 || !samples) {
        return {};
    }

    if(!enlargeInputBufferIfNeeded(numSamples)) {
        return {};
    }

    const auto buffer{m_inputBuffer.data() + m_numInputSamples * m_numChannels};
    const auto count{numSamples * m_numChannels};

    for (int i{};i < count;++i){
        buffer[i] = static_cast<int16_t>(samples[i] << 8);
    }

    m_numInputSamples += numSamples;
    return true;
}

void XSonic::removeInputSamples(const int &position) {

    const auto remainingSamples{m_numInputSamples - position};

    if(remainingSamples > 0) {
        const auto src_{m_inputBuffer.data() + position * m_numChannels};
        const auto dst_{m_inputBuffer.data()};
        const auto size_{remainingSamples * m_numChannels};
        std::move(src_,src_ + size_ ,dst_);
    }

    m_numInputSamples = remainingSamples;
}

bool XSonic::copyToOutput(const int16_t *samples,const int &numSamples) {

    if (!samples || numSamples <= 0){
        return {};
    }

    if(!enlargeOutputBufferIfNeeded(numSamples)) {
        return {};
    }

    const auto src_{samples};
    const auto dst_{m_outputBuffer.data() + m_numOutputSamples*m_numChannels};
    const auto size_{numSamples * m_numChannels};
    std::copy_n(src_,size_,dst_);

    m_numOutputSamples += numSamples;

    return true;
}

int XSonic::copyInputToOutput(const int &position) {

    auto numSamples{m_remainingInputToCopy};

    if(numSamples > m_maxRequired) {
        numSamples = m_maxRequired;
    }

    if(const auto in_buffer{m_inputBuffer.data() + position * m_numChannels};
        !copyToOutput(in_buffer,numSamples)) {
        return -1;
    }

    m_remainingInputToCopy -= numSamples;
    return numSamples;
}

void XSonic::downSampleInput(const int16_t *samples,
                             const int &skip) {

    if (!samples){
        return;
    }

    const auto numSamples{m_maxRequired / skip},
                samplesPerValue{m_numChannels * skip};

    auto downSamples{m_downSampleBuffer.data()};

    for(int i{}; i < numSamples; ++i) {
        int value{};
        for(int j {}; j < samplesPerValue; ++j) {
            value += *samples++;
        }
        value /= samplesPerValue;
        *downSamples++ = static_cast<int16_t>(value);
    }
}

int XSonic::findPitchPeriodInRange(const int16_t *samples,
                                   const int &minPeriod,
                                   const int &maxPeriod,
                                   int &retMinDiff,
                                   int &retMaxDiff) {
    if (!samples){
        return -1;
    }

    int worstPeriod{255},bestPeriod{},minDiff{1},maxDiff{};

    for(auto period{minPeriod}; period <= maxPeriod; period++) {
        int diff{};
        auto s{samples},p{samples + period};
        for(int i{}; i < period; i++) {
            const auto sVal{*s++},
                pVal{*p++};
            diff += sVal >= pVal? static_cast<int16_t>(sVal - pVal) :
                    static_cast<int16_t>(pVal - sVal);
        }
        /* Note that the highest number of samples we add into diff will be less
           than 256, since we skip samples.  Thus, diff is a 24 bit number, and
           we can safely multiply by numSamples without overflow */
        /* if (bestPeriod == 0 || (bestPeriod*3/2 > period && diff*bestPeriod < minDiff*period) ||
                diff*bestPeriod < (minDiff >> 1)*period) {*/
        if (!bestPeriod || diff * bestPeriod < minDiff * period) {
            minDiff = diff;
            bestPeriod = period;
        }

        if(diff * worstPeriod > maxDiff * period) {
            maxDiff = diff;
            worstPeriod = period;
        }
    }

    retMinDiff = minDiff / bestPeriod;
    retMaxDiff = maxDiff / worstPeriod;
    return bestPeriod;
}

bool XSonic::prevPeriodBetter(const int &minDiff,
                              const int &maxDiff,
                              const int &preferNewPeriod) const {

    if(!minDiff || !m_prevPeriod) {
        return {};
    }

    if(preferNewPeriod) {
        if(maxDiff > minDiff * 3) {
            /* Got a reasonable match this period */
            return {};
        }

        if(minDiff * 2 <= m_prevMinDiff * 3) {
            /* Mismatch is not that much greater this period */
            return {};
        }
    } else {
        if(minDiff <= m_prevMinDiff) {
            return {};
        }
    }

    return true;
}

int XSonic::findPitchPeriod(const int16_t *samples,const int &preferNewPeriod) {

    if (!samples){
        return -1;
    }

    auto minPeriod{m_minPeriod},maxPeriod{m_maxPeriod};
    const auto sampleRate{m_sampleRate};

    auto skip {1};
    if(sampleRate > SONIC_AMDF_FREQ_ && !m_quality ) {
        skip = sampleRate / SONIC_AMDF_FREQ_;
    }

    int period,minDiff{},maxDiff{};
    if(1 == m_numChannels && 1 == skip) {
        period = findPitchPeriodInRange(samples, minPeriod, maxPeriod, minDiff, maxDiff);
    } else {
        downSampleInput(samples, skip);
        period = findPitchPeriodInRange(m_downSampleBuffer.data(), minPeriod/skip,
                                        maxPeriod/skip, minDiff, maxDiff);
        if(1 != skip) {
            period *= skip;
            minPeriod = period - (skip << 2);
            maxPeriod = period + (skip << 2);

            if(minPeriod < m_minPeriod) {
                minPeriod = m_minPeriod;
            }

            if(maxPeriod > m_maxPeriod) {
                maxPeriod = m_maxPeriod;
            }

            if(1 == m_numChannels) {
                period = findPitchPeriodInRange(samples, minPeriod, maxPeriod,
                                                minDiff, maxDiff);
            } else {
                downSampleInput(samples, 1);
                period = findPitchPeriodInRange(m_downSampleBuffer.data(), minPeriod,
                                                maxPeriod, minDiff, maxDiff);
            }
        }
    }

    const auto retPeriod{prevPeriodBetter(minDiff, maxDiff, preferNewPeriod) ?
                         m_prevPeriod : period};

    m_prevMinDiff = minDiff;
    m_prevPeriod = period;
    return retPeriod;
}

void XSonic::overlapAdd(const int &numSamples,
                        const int &numChannels,
                        int16_t *out,
                        const int16_t *rampDown,
                        const int16_t *rampUp) {

    for(uint32_t i{};i < numChannels;i++) {

        auto o{out + i};
        auto u{rampUp + i},d{rampDown + i};

        for(int t{}; t < numSamples; t++) {
            const auto v_d{*d},v_u{*u};
#ifdef SONIC_USE_SIN
            float ratio = sin(t*M_PI/(2*numSamples));
            *o = v_d * (1.0f - ratio) + v_u * ratio;
#else
            *o = static_cast<int16_t>((v_d * (numSamples - t) + v_u *t) / numSamples);
#endif
            o += numChannels;
            d += numChannels;
            u += numChannels;
        }
    }
}

void XSonic::overlapAddWithSeparation(const int &numSamples,
                                      const int &numChannels,
                                      const int &separation,
                                      int16_t *out,
                                      const int16_t *rampDown,
                                      const int16_t *rampUp) {

    for(uint32_t i{}; i < numChannels; i++) {

        auto o{out + i};
        auto u{rampUp + i},d{rampDown + i};

        for(int t{}; t < numSamples + separation; t++) {

            const auto v_d{*d},v_u{*u};

            if(t < separation) {
                *o = static_cast<int16_t>(v_d * (numSamples - t) / numSamples);
                d += numChannels;
            } else if(t < numSamples) {
                *o = static_cast<int16_t>((v_d * (numSamples - t) + v_u * (t - separation)) / numSamples);
                d += numChannels;
                u += numChannels;
            } else {
                *o = static_cast<int16_t>(v_u * (t - separation) / numSamples);
                u += numChannels;
            }
            o += numChannels;
        }
    }
}

bool XSonic::moveNewSamplesToPitchBuffer(const int &originalNumOutputSamples) {

    const auto numSamples{m_numOutputSamples - originalNumOutputSamples},
                numChannels{m_numChannels};

    if(m_numPitchSamples + numSamples > m_pitchBufferSize) {
        m_pitchBufferSize += (m_pitchBufferSize >> 1) + numSamples;
        m_pitchBuffer.clear();
        TRY_CATCH(CHECK_EXC(m_pitchBuffer.resize(m_pitchBufferSize * numChannels)),return {});
    }

    const auto src_{m_outputBuffer.data() + originalNumOutputSamples * numChannels};
    const auto dst_{m_pitchBuffer.data() + m_numPitchSamples * numChannels};
    const auto size_{numSamples * numChannels};
    std::copy_n(src_,size_,dst_);

    m_numOutputSamples = originalNumOutputSamples;
    m_numPitchSamples += numSamples;
    return true;
}

void XSonic::removePitchSamples(const int &numSamples) {

    if(!numSamples) {
        return;
    }

    if(numSamples != m_numPitchSamples) {
        const auto numChannels{m_numChannels};
        const auto src_{m_pitchBuffer.data() + numSamples * numChannels};
        const auto dst_{m_pitchBuffer.data()};
        const auto size_{(m_numPitchSamples - numSamples) * numChannels};
        std::move(src_,src_ + size_,dst_);
    }

    m_numPitchSamples -= numSamples;
}

bool XSonic::adjustPitch(const int &originalNumOutputSamples) {

    const auto pitch{m_pitch};
    const auto numChannels{m_numChannels};

    if(originalNumOutputSamples == m_numOutputSamples) {
        return true;
    }

    if(!moveNewSamplesToPitchBuffer(originalNumOutputSamples)) {
        return {};
    }

    int position {};
    while(m_numPitchSamples - position >= m_maxRequired) {
        const auto period{findPitchPeriod(m_pitchBuffer.data() + position * numChannels, 0)};

        const auto float_period{static_cast<float >(period)};
        const auto newPeriod{static_cast<int>(float_period / pitch)};

        if(!enlargeOutputBufferIfNeeded( newPeriod)) {
            return {};
        }

        const auto out{m_outputBuffer.data() + m_numOutputSamples * numChannels};

        if(pitch >= 1.0f) {
            const auto rampDown{m_pitchBuffer.data() + position * numChannels},
                rampUp{m_pitchBuffer.data() + (position + period - newPeriod) * numChannels};
            overlapAdd(newPeriod, numChannels, out, rampDown, rampUp);
        } else {
            const auto rampDown{m_pitchBuffer.data() + position * numChannels},
                rampUp{m_pitchBuffer.data() + position * numChannels};
            const auto separation{newPeriod - period};
            overlapAddWithSeparation(period, numChannels, separation, out, rampDown, rampUp);
        }

        m_numOutputSamples += newPeriod;
        position += period;
    }

    removePitchSamples(position);
    return true;
}

int XSonic::findSincCoefficient(const int &i,
                                const int &ratio,
                                const int &width) {

    static constexpr auto lobePoints {(SINC_TABLE_SIZE - 1) / SINC_FILTER_POINTS};

    const auto left{i * lobePoints + (ratio * lobePoints) / width},
                right{left + 1};

    const auto position{static_cast<int>(i * lobePoints * width + ratio * lobePoints - left * width)};

    const auto leftVal{sincTable[left]},
                rightVal{sincTable[right]};

    return ((leftVal * (width - position) + rightVal * position) << 1) / width;
}

int16_t XSonic::interpolate(const int16_t *in,
                          const int &oldSampleRate,
                          const int &newSampleRate) const {

    if (!in){
        return -1;
    }

    constexpr auto getSign{[](const int &value){
        return value >= 0;
    }};

    /* Compute N-point sinc FIR-filter here.  Clip rather than overflow. */

    const auto position{m_newRatePosition * oldSampleRate},
                leftPosition{m_oldRatePosition * newSampleRate},
                rightPosition{(m_oldRatePosition + 1) * newSampleRate},
                ratio{rightPosition - position - 1},
                width{rightPosition - leftPosition};

    int total{},overflowCount{};
    for (int i{}; i < SINC_FILTER_POINTS; i++) {

        const auto weight{findSincCoefficient(i, ratio, width)};
        /* printf("%u %f\n", i, weight); */
        const auto value{in[i * m_numChannels] * weight};
        const auto oldSign {getSign(total)};
        total += value;

        if (oldSign != getSign(total) && getSign(value) == oldSign) {
            /* We must have overflowed.  This can happen with a sinc filter. */
            overflowCount += oldSign;
        }
    }

    /* It is better to clip than to wrap if there was a overflow. */
    if (overflowCount > 0) {
        return SHRT_MAX;
    }

    if(overflowCount < 0) {
        return SHRT_MIN;
    }

    return static_cast<int16_t>(total >> 16);
}

bool XSonic::adjustRate(const float &rate,const int &originalNumOutputSamples) {

    auto oldSampleRate{m_sampleRate},
        newSampleRate{static_cast<int>(static_cast<float>(oldSampleRate) / rate)},
        numChannels{m_numChannels};

    static constexpr auto N{SINC_FILTER_POINTS};
    /* Set these values to help with the integer math */
    while(newSampleRate > 1 << 14 || oldSampleRate > 1 << 14) {
        newSampleRate >>= 1;
        oldSampleRate >>= 1;
    }

    if(originalNumOutputSamples == m_numOutputSamples) {
        return true;
    }

    if(!moveNewSamplesToPitchBuffer(originalNumOutputSamples)) {
        return {};
    }

    /* Leave at least N pitch sample in the buffer */
    int position{};
    for(;position < m_numPitchSamples - N; position++) {

        while((m_oldRatePosition + 1) * newSampleRate > m_newRatePosition * oldSampleRate) {

            if(!enlargeOutputBufferIfNeeded(1)) {
                return {};
            }

            auto out{m_outputBuffer.data() + m_numOutputSamples * numChannels},
                in{m_pitchBuffer.data() + position * numChannels};

            for(int i{}; i < numChannels; i++) {
                *out++ = interpolate(in++, oldSampleRate, newSampleRate);
            }

            ++m_newRatePosition;
            ++m_numOutputSamples;
        }

        m_oldRatePosition++;

        if(m_oldRatePosition == oldSampleRate) {

            m_oldRatePosition = 0;

            if(m_newRatePosition != newSampleRate) {
                std::cerr << "Assertion failed: m_newRatePosition != newSampleRate\n";
                return {};
            }

            m_newRatePosition = 0;
        }
    }

    removePitchSamples(position);
    return true;
}

int XSonic::skipPitchPeriod(const int16_t *samples,
                            const float &speed,
                            const int &period) {

    const auto numChannels{m_numChannels};
    const auto f_period{static_cast<float >(period)};

    int newSamples;
    if(speed >= 2.0f) {
        newSamples = static_cast<decltype(newSamples) >(f_period / (speed - 1.0f));
    } else {
        newSamples = period;
        m_remainingInputToCopy = static_cast<decltype(m_remainingInputToCopy)>(f_period * (2.0f - speed) / (speed - 1.0f));
    }

    if(!enlargeOutputBufferIfNeeded(newSamples)) {
        return -1;
    }

    const auto out_buffer{m_outputBuffer.data() + m_numOutputSamples * numChannels};

    overlapAdd(newSamples,numChannels,out_buffer,samples,samples + period * numChannels);

    m_numOutputSamples += newSamples;

    return newSamples;
}

int XSonic::insertPitchPeriod(const int16_t *samples,
                              const float &speed,
                              const int &period) {

    const auto numChannels{m_numChannels};
    const auto f_period{static_cast<float>(period)};

    int newSamples;
    if(speed < 0.5f) {
        newSamples = static_cast<decltype(newSamples)>(f_period * speed / (1.0f - speed));
    } else {
        newSamples = period;
        m_remainingInputToCopy = static_cast<decltype(m_remainingInputToCopy)>(f_period * (2.0f * speed - 1.0f) / (1.0f - speed));
    }

    if(!enlargeOutputBufferIfNeeded(period + newSamples)) {
        return -1;
    }

    auto out{m_outputBuffer.data() + m_numOutputSamples * numChannels};

    std::copy_n(samples,period * numChannels,out);

    out = m_outputBuffer.data() + (m_numOutputSamples + period) * numChannels;

    overlapAdd(newSamples,numChannels,out,samples + period * numChannels,samples);

    m_numOutputSamples += period + newSamples;

    return newSamples;
}

bool XSonic::changeSpeed(const float &speed) {

    const auto numSamples{m_numInputSamples},
                maxRequired{m_maxRequired};

    if(m_numInputSamples < maxRequired) {
        return true;
    }

    int position{};
    do {
        int newSamples;
        if(m_remainingInputToCopy > 0) {
            newSamples = copyInputToOutput(position);

            if (newSamples < 0){
                return {};
            }

            position += newSamples;
        } else {

            const auto samples{m_inputBuffer.data() + position * m_numChannels};
            const auto period{findPitchPeriod(samples, 1)};

            if (!period) {
                return {};
            }

            if(speed > 1.0) {
                newSamples = skipPitchPeriod(samples,speed,period);

                if (!newSamples){
                    return {};
                }

                position += period + newSamples;
            } else {
                newSamples = insertPitchPeriod( samples, speed, period);

                if (!newSamples) {
                    return {};
                }

                position += newSamples;
            }
        }

        if(!newSamples) {
            return {}; /* Failed to resize output buffer */
        }

    } while(position + maxRequired <= numSamples);

    removeInputSamples(position);

    return true;
}

bool XSonic::processStreamInput() {

    const auto originalNumOutputSamples{m_numOutputSamples};
    const auto speed{m_speed / m_pitch};
    auto rate{m_rate};

    if(!m_useChordPitch) {
        rate *= m_pitch;
    }

    if(speed > 1.00001f || speed < 0.99999f) {
        changeSpeed(speed);
    } else {
        if(!copyToOutput(m_inputBuffer.data(),m_numInputSamples)) {
            return {};
        }
        m_numInputSamples = 0;
    }

    if(m_useChordPitch) {
        if(1.0f != m_pitch) {
            if(!adjustPitch(originalNumOutputSamples)) {
                return {};
            }
        }
    } else if(1.0f != rate) {
        if(!adjustRate(rate,originalNumOutputSamples)) {
            return {};
        }
    }else{}

    if(1.0f != m_volume) {
        /* Adjust output volume. */
        scaleSamples(m_outputBuffer.data() + originalNumOutputSamples*m_numChannels,
                     (m_numOutputSamples - originalNumOutputSamples)*m_numChannels,
                     m_volume);
    }

    return true;
}

void XSonic::Move_(XSonic *src) noexcept(true){
    m_inputBuffer = std::move(src->m_inputBuffer);
    m_outputBuffer = std::move(src->m_outputBuffer);
    m_pitchBuffer = std::move(src->m_pitchBuffer);
    m_downSampleBuffer = std::move(src->m_downSampleBuffer);
    const auto dst_{static_cast<XSonic_data*>(this)},
        src_{static_cast<XSonic_data*>(src)};
    *dst_ = *src_;
    *src_ = {};
}
