#include "FlvMetaData.h"
#include <cstring>
#include <iostream>
#include <string>
#include <sstream>

void FlvMetaData::Shallow_copy(const FlvMetaData& r) noexcept{
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

void FlvMetaData::_Clear_Rvalue(FlvMetaData&& r) noexcept {
    r.m_length = 0;
    r.m_meta = nullptr;
    r.m_duration = 0;
    r.m_width = 0;
    r.m_height = 0;
    r.m_framerate = 0;
    r.m_videodatarate = 0;
    r.m_audiodatarate = 0;
    r.m_videocodecid = 0;
    r.m_audiosamplerate = 0;
    r.m_audiosamplesize = 0;
    r.m_audiocodecid = 0;
    r.m_stereo = false;
}

void FlvMetaData::_Copy_construct_from(const FlvMetaData& r) noexcept{
    Shallow_copy(r);
    m_meta = new uint8_t[m_length];
    std::copy_n(r.m_meta, r.m_length,m_meta);
}

void FlvMetaData::_Move_construct_from(FlvMetaData&& r) noexcept{

    Shallow_copy(r);
    m_meta = r.m_meta;
    _Clear_Rvalue(std::move(r));
}

void FlvMetaData::_Swap(FlvMetaData &r) {
    std::swap(m_meta,r.m_meta);
    std::swap(m_length,r.m_length);
    std::swap(m_duration ,r.m_duration);
    std::swap(m_width , r.m_width);
    std::swap(m_height , r.m_height);
    std::swap(m_framerate , r.m_framerate);
    std::swap(m_videodatarate , r.m_videodatarate);
    std::swap(m_audiodatarate , r.m_audiodatarate);
    std::swap(m_videocodecid , r.m_videocodecid);
    std::swap(m_audiosamplerate , r.m_audiosamplerate);
    std::swap(m_audiosamplesize ,r.m_audiosamplesize);
    std::swap(m_audiocodecid , r.m_audiocodecid);
    std::swap(m_stereo , r.m_stereo);
}

FlvMetaData::FlvMetaData(const uint8_t *meta,const uint32_t length):
m_meta{new uint8_t[length]},m_length{length}
{
    std::copy_n(meta,m_length,m_meta);
    parseMeta();
}

FlvMetaData::FlvMetaData(const FlvMetaData& r) {
    _Copy_construct_from(r);
}

FlvMetaData::FlvMetaData(FlvMetaData&& r) noexcept
{
    _Move_construct_from(std::move(r));
}

FlvMetaData& FlvMetaData::operator=(const FlvMetaData& r) {
    FlvMetaData(r)._Swap(*this);
    return *this;
}

FlvMetaData& FlvMetaData::operator=(FlvMetaData&& r) noexcept{
    FlvMetaData(std::move(r))._Swap(*this);
    return *this;
}

FlvMetaData::~FlvMetaData() {
    delete[] m_meta;
    m_meta = nullptr;
}

void FlvMetaData::parseMeta() {

    constexpr auto TAG_HEAD_LEN{11};
    //unsigned int offset = TAG_HEAD_LEN + 13;
    uint32_t offset {13},arrayLen{};

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
        std::cerr << "metadata format error!!!\n";
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

        //char name[_nameLen + 1]{};
        const auto name{new char[nameLen + 1]};
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
        std::cout << "name = " << name << "\n" << std::flush;
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
        delete[]name;
    }
}

double FlvMetaData::hexStr2double(const uint8_t* hex, const uint32_t length) {

    const auto size{length * 2};
    char hexstr[size];
    std::fill_n(hexstr,length * 2,0);
    for(uint32_t i {}; i < length; i++) {
        sprintf(hexstr + i * 2, "%02x", hex[i]);
    }

    double ret {};
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
