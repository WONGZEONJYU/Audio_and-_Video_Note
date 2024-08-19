#include "sonic.hpp"

bool XSonic::AllocateStreamBuffers(const int &sampleRate,const int &numChannels) {


    const auto minPeriod {sampleRate / SONIC_MAX_PITCH};
    const auto maxPeriod {sampleRate / SONIC_MIN_PITCH};
    const auto maxRequired{2 * m_maxPeriod};

    m_inputBufferSize = maxRequired;
    m_inputBuffer.clear();
    m_inputBuffer.resize(maxRequired * numChannels);

    m_outputBufferSize = maxRequired;
    m_outputBuffer.clear();
    m_outputBuffer.resize(maxRequired * numChannels);

    m_pitchBufferSize = maxRequired;
    m_pitchBuffer.clear();
    m_pitchBuffer.resize(maxRequired * numChannels);

    m_downSampleBuffer.clear();
    m_downSampleBuffer.resize(maxRequired);

    m_sampleRate = sampleRate;
    m_numChannels = numChannels;
    m_prevPeriod = m_oldRatePosition = m_newRatePosition = 0;
    m_minPeriod = m_maxPeriod = maxRequired;

    return true;
}

void XSonic::Open(const int &sampleRate,const int &numChannels){

}

int XSonic::sonicWriteFloatToStream(float *samples, int numSamples) {
    return 0;
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

void XSonic::sonicSetSampleRate(int sampleRate) {

}

int XSonic::sonicGetNumChannels() {
    return 0;
}

void XSonic::sonicSetNumChannels(int numChannels) {

}

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

