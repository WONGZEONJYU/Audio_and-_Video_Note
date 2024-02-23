#include "FlvMetaData.h"
#include <cstring>
#include <iostream>

using namespace std;

void FlvMetaData::Shallow_copy(const FlvMetaData& r)
{
    m_length = r.m_length;
    m_duration = r.m_duration;
    m_width = r.m_width;
    m_height = r.m_height;
    m_framerate = r.m_framerate;
    m_videodatarate = r.m_videodatarate;
    m_audiodatarate = r.m_audiodatarate;
    m_videocodecid = r.m_videocodecid;
    m_audiosamplerate = r.m_audiosamplerate;
    m_audiosamplesize = r.m_audiosamplesize;
    m_audiocodecid = r.m_audiocodecid;
    m_stereo = r.m_stereo;
}

void FlvMetaData::_copy_(const FlvMetaData& r)
{
    Shallow_copy(r);
    m_meta = new uint8_t[m_length];
    copy(r.m_meta,r.m_meta + r.m_length,m_meta);
}

void FlvMetaData::_shift_(FlvMetaData&& r,FlvMetaData&& t)
{
    Shallow_copy(r);
    t.m_meta = m_meta;
    m_meta = r.m_meta;
}

FlvMetaData::FlvMetaData(const uint8_t *meta,const uint32_t length):
m_meta{new uint8_t[length]},m_length{length}
{
    copy(meta,meta + m_length,m_meta);
    parseMeta();
}

FlvMetaData::FlvMetaData(const FlvMetaData& r) {
    _copy_(r);
}

FlvMetaData::FlvMetaData(FlvMetaData&& r) noexcept
{
    FlvMetaData temp;
    _shift_(forward<decltype(r)>(r),forward<decltype(temp)>(temp));
}

FlvMetaData& FlvMetaData::operator=(const FlvMetaData& r) {

    if(this != &r) {
        delete[] m_meta;
        _copy_(r);
    }
    return *this;
}

FlvMetaData& FlvMetaData::operator=(FlvMetaData&& r) noexcept
{
    if (this != &r){
        FlvMetaData temp;
        _shift_(forward<decltype(r)>(r),forward<decltype(temp)>(temp));
    }
    return *this;
}

FlvMetaData::~FlvMetaData() {
    delete[] m_meta;
    m_meta = nullptr;
}

void FlvMetaData::parseMeta() {

    uint32_t arrayLen{};
    constexpr auto TAG_HEAD_LEN{11};
    //unsigned int offset = TAG_HEAD_LEN + 13;
    uint32_t offset {13};

    if(m_meta[offset++] == 0x08) {

        arrayLen |= m_meta[offset++];
        arrayLen <<= 8;
        arrayLen |= m_meta[offset++];
        arrayLen <<= 8;
        arrayLen |= m_meta[offset++];
        arrayLen <<= 8;
        arrayLen |= m_meta[offset++];

        //cerr << "ArrayLen = " << arrayLen << endl;
    } else {
        //TODO:
        cerr << "metadata format error!!!\n";
        return ;
    }

    for(uint32_t i {}; i < arrayLen; i++) {

        double numValue {};
        bool boolValue {};

        uint32_t nameLen {};
        nameLen |= m_meta[offset++];
        nameLen = nameLen << 8;
        nameLen |= m_meta[offset++];
        //cerr << "name length=" << nameLen << " ";

        char name[nameLen + 1]{};
#if DEBUG
        printf("\norign \n");
        for(uint32_t i {}; i < nameLen + 3; i++) {
            printf("%x ", m_meta[offset + i]);
        }
        printf("\n");
#endif

        memcpy(name, &m_meta[offset], nameLen);
        name[nameLen + 1] = '\0';
        offset += nameLen;
        cout << "name = " << name << "\n" << flush;
        //cerr << "name=" << name << " ";
#if DEBUG
        printf("memcpy\n");
        for(uint32_t i {}; i < nameLen; i++) {
            printf("%x ", name[i]);
        }
        printf("\n");
#endif
        switch(m_meta[offset++]) {
        case 0x0: //Number type
            numValue = hexStr2double(&m_meta[offset], 8);
            offset += 8;
            break;

        case 0x1: //Boolean type
            if(offset++ != 0x00) {
                boolValue = true;
            }
            break;

        case 0x2: //String type
            nameLen = 0;
            nameLen |= m_meta[offset++];
            nameLen = nameLen << 8;
            nameLen |= m_meta[offset++];
            offset += nameLen;
            break;

        case 0x12: //Long string type
            nameLen = 0;
            nameLen |= m_meta[offset++];
            nameLen = nameLen << 8;
            nameLen |= m_meta[offset++];
            nameLen = nameLen << 8;
            nameLen |= m_meta[offset++];
            nameLen = nameLen << 8;
            nameLen |= m_meta[offset++];
            offset += nameLen;
            break;

        //FIXME:
        default:
            break;
        }

        if(!strncmp(name, "duration", 8)) {
            m_duration = numValue;
        } else if(strncmp(name, "width", 5)	== 0) {
            m_width = numValue;
        } else if(strncmp(name, "height", 6) == 0) {
            m_height = numValue;
        } else if(strncmp(name, "framerate", 9) == 0) {
            m_framerate = numValue;
        } else if(strncmp(name, "videodatarate", 13) == 0) {
            m_videodatarate = numValue;
        } else if(strncmp(name, "audiodatarate", 13) == 0) {
            m_audiodatarate = numValue;
        } else if(strncmp(name, "videocodecid", 12) == 0) {
            m_videocodecid = numValue;
        } else if(strncmp(name, "audiosamplerate", 15) == 0) {
            m_audiosamplerate = numValue;
        } else if(strncmp(name, "audiosamplesize", 15) == 0) {
            m_audiosamplesize = numValue;
        } else if(strncmp(name, "audiocodecid", 12) == 0) {
            m_audiocodecid = numValue;
        } else if(strncmp(name, "stereo", 6) == 0) {
            m_stereo = boolValue;
        }
    }
}

double FlvMetaData::hexStr2double(const uint8_t* hex, const uint32_t length) {

    double ret {};
    char hexstr [length * 2]{};

    for(uint32_t i {}; i < length; i++) {
        sprintf(hexstr + i * 2, "%02x", hex[i]);
    }

    sscanf(hexstr, "%llx", reinterpret_cast<uint64_t*>(&ret));
    return ret;
}

double FlvMetaData::getDuration() const {
    return m_duration;
}

double FlvMetaData::getWidth() const{
    return m_width;
}

double FlvMetaData::getHeight() const{
    return m_height;
}

double FlvMetaData::getFramerate() const{
    return m_framerate;
}

double FlvMetaData::getVideoDatarate() const{
    return m_videodatarate;
}

double FlvMetaData::getAudioDatarate() const{
    return m_audiodatarate;
}

double FlvMetaData::getVideoCodecId() const{
    return m_videocodecid;
}

double FlvMetaData::getAudioSamplerate() const{
    return m_audiosamplerate;
}

double FlvMetaData::getAudioSamplesize() const{
    return m_audiosamplesize;
}

double FlvMetaData::getAudioCodecId() const{
    return m_audiocodecid;
}

bool FlvMetaData::getStereo() const{
    return m_stereo;
}
