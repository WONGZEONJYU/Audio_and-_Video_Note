#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <memory>
#include "FlvParser.h"

using namespace std;

#define CheckBuffer(x) { if ((nBufSize-nOffset)<(x)) { nUsedLen = nOffset; return 0;} }

int CFlvParser::CAudioTag::_aacProfile{};
int CFlvParser::CAudioTag::_sampleRateIndex{};
int CFlvParser::CAudioTag::_channelConfig{};

static constexpr uint32_t nH264StartCode {0x01000000};

CFlvParser::CFlvParser():_vjj(new CVideojj())
{
}

CFlvParser::~CFlvParser(){

    for (int i {}; i < _vpTag.size(); i++){
        DestroyTag(_vpTag[i]);
        delete _vpTag[i];
    }
}

int CFlvParser::Parse(const uint8_t *pBuf,const int nBufSize, int &nUsedLen)
{
    int nOffset {};

    if (!_pFlvHeader){
        CheckBuffer(9);
        _pFlvHeader = CreateFlvHeader(pBuf+nOffset);
        nOffset += _pFlvHeader->nHeadSize;
    }

    for(;;) {
        CheckBuffer(15); // nPrevSize(4字节) + Tag header(11字节)
        auto nPrevSize { ShowU32(pBuf + nOffset)};
        nOffset += 4;

        auto pTag {CreateTag(pBuf + nOffset, nBufSize - nOffset)};
        if (!pTag){
            nOffset -= 4;
            break;
        }
        nOffset += (11 + pTag->_header.nDataSize);

        _vpTag.push_back(pTag);
    }

    nUsedLen = nOffset;
    return 0;
}

int CFlvParser::PrintInfo()
{
    Stat();
    cout << "vnum: " << _sStat.nVideoNum << " , anum: " << _sStat.nAudioNum << " , mnum: " << _sStat.nMetaNum << "\n";
    cout << "maxTimeStamp: " << _sStat.nMaxTimeStamp << " ,nLengthSize: " << _sStat.nLengthSize << "\n";
    cout << "Vjj SEI num: " << _vjj->_vVjjSEI.size() << endl;
    for (uint32_t i {}; i < _vjj->_vVjjSEI.size(); i++)
        cout << "SEI time : " << _vjj->_vVjjSEI[i].nTimeStamp << endl;
    return 1;
}

int CFlvParser::DumpH264(const std::string &path) const
{
    fstream f(path.c_str(), ios_base::out|ios_base::binary);

    for(const auto &it_tag:_vpTag) {
        if ( 0x09 != it_tag->_header.nType ) {
            continue;
        }
        f.write(reinterpret_cast<char *>(it_tag->_pMedia), it_tag->_nMediaLen);
    }

    f.close();

    return 1;
}

int CFlvParser::DumpAAC(const std::string &path)
{
    fstream f;
    f.open(path.c_str(), ios_base::out | ios_base::binary);

    // vector<Tag *>::iterator it_tag;
    // for (it_tag = _vpTag.begin(); it_tag != _vpTag.end(); it_tag++)
    // {
    //     if ((*it_tag)->_header.nType != 0x08)
    //         continue;
    //
    //     CAudioTag *pAudioTag = (CAudioTag *)(*it_tag);
    //     if (pAudioTag->_nSoundFormat != 10)
    //         continue;
    //
    //     if (pAudioTag->_nMediaLen!=0)
    //         f.write((char *)(*it_tag)->_pMedia, (*it_tag)->_nMediaLen);
    // }

    for (const auto &it_tag : _vpTag) {
        if (0x08 != it_tag->_header.nType) {
            continue;
        }

        const auto pAudioTag {reinterpret_cast<CAudioTag*>(it_tag)};

        if ( 10 != pAudioTag->_nSoundFormat ) {
            continue;
        }

        if (pAudioTag->_nMediaLen) {
            f.write(reinterpret_cast<const char*>(it_tag->_pMedia),
                    it_tag->_nMediaLen);
        }
    }
    f.close();

    return 1;
}

int CFlvParser::DumpFlv(const std::string &path)
{
    fstream f(path.c_str(), ios_base::out | ios_base::binary);

    // write flv-header
    f.write(reinterpret_cast<char *>(_pFlvHeader->pFlvHeader), _pFlvHeader->nHeadSize);
    uint32_t nLastTagSize {};

    // write flv-tag

    for (const auto &it_tag : _vpTag)
    {
        const auto nn { WriteU32(nLastTagSize)};
        f.write(reinterpret_cast<const char *>(&nn), 4);

        //check duplicate start code
        if ((0x09 == it_tag->_header.nType) && (0x01 == it_tag->_pTagData[1]) ) {
            bool duplicate {};
            const auto pStartCode  {it_tag->_pTagData + 5 + _nNalUnitLength};
            //printf("tagsize=%d\n",(*it_tag)->_header.nDataSize);
            uint32_t nalu_len {};
            const auto p_nalu_len { reinterpret_cast<uint8_t*>(&nalu_len)};
            switch (_nNalUnitLength) {
            case 4:
                nalu_len = ShowU32(it_tag->_pTagData + 5);
                break;
            case 3:
                nalu_len = ShowU24(it_tag->_pTagData + 5);
                break;
            case 2:
                nalu_len = ShowU16(it_tag->_pTagData + 5);
                break;
            default:
                nalu_len = ShowU8(it_tag->_pTagData + 5);
                break;
            }
            /*
            printf("nalu_len=%u\n",nalu_len);
            printf("%x,%x,%x,%x,%x,%x,%x,%x,%x\n",(*it_tag)->_pTagData[5],(*it_tag)->_pTagData[6],
                    (*it_tag)->_pTagData[7],(*it_tag)->_pTagData[8],(*it_tag)->_pTagData[9],
                    (*it_tag)->_pTagData[10],(*it_tag)->_pTagData[11],(*it_tag)->_pTagData[12],
                    (*it_tag)->_pTagData[13]);
            */

            const auto pStartCodeRecord {pStartCode};
            int i{};
            for ( ; i < it_tag->_header.nDataSize - 5 - _nNalUnitLength - 4; ++i) {
                if (pStartCode[i] == 0x00 && pStartCode[i+1] == 0x00 && pStartCode[i+2] == 0x00 &&
                        pStartCode[i+3] == 0x01) {
                    if (pStartCode[i+4] == 0x67) {
                        //printf("duplicate sps found!\n");
                        i += 4;
                        continue;
                    }else if (pStartCode[i+4] == 0x68) {
                        //printf("duplicate pps found!\n");
                        i += 4;
                        continue;
                    }else if (pStartCode[i+4] == 0x06) {
                        //printf("duplicate sei found!\n");
                        i += 4;
                        continue;
                    }else {
                        i += 4;
                        //printf("offset=%d\n",i);
                        duplicate = true;
                        break;
                    }
                }
            }

            if (duplicate) {
                nalu_len -= i;
                it_tag->_header.nDataSize -= i;
                const auto p { reinterpret_cast<uint8_t *>(&it_tag->_header.nDataSize)};
                it_tag->_pTagHeader[1] = p[2];
                it_tag->_pTagHeader[2] = p[1];
                it_tag->_pTagHeader[3] = p[0];
                //printf("after,tagsize=%d\n",(int)ShowU24((*it_tag)->_pTagHeader + 1));
                //printf("%x,%x,%x\n",(*it_tag)->_pTagHeader[1],(*it_tag)->_pTagHeader[2],(*it_tag)->_pTagHeader[3]);

                f.write(reinterpret_cast<char*>(it_tag->_pTagHeader), 11);
                switch (_nNalUnitLength) {
                case 4:
                    *(it_tag->_pTagData + 5) = p_nalu_len[3];
                    *(it_tag->_pTagData + 6) = p_nalu_len[2];
                    *(it_tag->_pTagData + 7) = p_nalu_len[1];
                    *(it_tag->_pTagData + 8) = p_nalu_len[0];
                    break;
                case 3:
                    *(it_tag->_pTagData + 5) = p_nalu_len[2];
                    *(it_tag->_pTagData + 6) = p_nalu_len[1];
                    *(it_tag->_pTagData + 7) = p_nalu_len[0];
                    break;
                case 2:
                    *(it_tag->_pTagData + 5) = p_nalu_len[1];
                    *(it_tag->_pTagData + 6) = p_nalu_len[0];
                    break;
                default:
                    *(it_tag->_pTagData + 5) = p_nalu_len[0];
                    break;
                }
                //printf("after,nalu_len=%d\n",(int)ShowU32((*it_tag)->_pTagData + 5));
                f.write(reinterpret_cast<const char *>(it_tag->_pTagData), pStartCode - it_tag->_pTagData);
                /*
                printf("%x,%x,%x,%x,%x,%x,%x,%x,%x\n",(*it_tag)->_pTagData[0],(*it_tag)->_pTagData[1],(*it_tag)->_pTagData[2],
                        (*it_tag)->_pTagData[3],(*it_tag)->_pTagData[4],(*it_tag)->_pTagData[5],(*it_tag)->_pTagData[6],
                        (*it_tag)->_pTagData[7],(*it_tag)->_pTagData[8]);
                */
                f.write(reinterpret_cast<const char *>(pStartCode + i), it_tag->_header.nDataSize - (pStartCode - it_tag->_pTagData));
                /*
                printf("write size:%d\n", (pStartCode - (*it_tag)->_pTagData) +
                        ((*it_tag)->_header.nDataSize - (pStartCode - (*it_tag)->_pTagData)));
                */
            } else {
                f.write(reinterpret_cast<const char *>(it_tag->_pTagHeader), 11);
                f.write(reinterpret_cast<const char *>(it_tag->_pTagData), it_tag->_header.nDataSize);
            }
        } else {
            f.write(reinterpret_cast<const char *>(it_tag->_pTagHeader), 11);
            f.write(reinterpret_cast<const char *> (it_tag->_pTagData), it_tag->_header.nDataSize);
        }

        nLastTagSize = 11 + it_tag->_header.nDataSize;
    }
    const auto nn {WriteU32(nLastTagSize)};
    f.write(reinterpret_cast<const char *>(&nn), 4);

    f.close();

    return 1;
}

int CFlvParser::Stat()
{
    // for (int i {}; i < _vpTag.size(); i++){
    //     switch (_vpTag[i]->_header.nType)
    //     {
    //     case 0x08:
    //         _sStat.nAudioNum++;
    //         break;
    //     case 0x09:
    //         StatVideo(_vpTag[i]);
    //         break;
    //     case 0x12:
    //         _sStat.nMetaNum++;
    //         break;
    //     default:
    //         ;
    //     }
    // }

    for(const auto &i : _vpTag){
        switch (i->_header.nType) {
            case 0x08:
                _sStat.nAudioNum++;
                break;
            case 0x09:
                StatVideo(i);
                break;
            case 0x12:
                _sStat.nMetaNum++;
                break;
            default:
                break;
        }
    }

    return 1;
}

int CFlvParser::StatVideo(const Tag * const pTag)
{
    _sStat.nVideoNum++;
    _sStat.nMaxTimeStamp = pTag->_header.nTimeStamp;

    if (pTag->_pTagData[0] == 0x17 && pTag->_pTagData[1] == 0x00){
        _sStat.nLengthSize = (pTag->_pTagData[9] & 0x03) + 1;
    }

    return 1;
}

CFlvParser::FlvHeader* CFlvParser::CreateFlvHeader(const uint8_t *pBuf)
{
    const auto pHeader { new FlvHeader};
    pHeader->nVersion = pBuf[3];        // 版本号
    pHeader->bHaveAudio = (pBuf[4] >> 2) & 0x01;    // 是否有音频
    pHeader->bHaveVideo = (pBuf[4] >> 0) & 0x01;    // 是否有视频
    pHeader->nHeadSize = static_cast<int>(ShowU32(pBuf + 5));         // 头部长度

    pHeader->pFlvHeader = new uint8_t[pHeader->nHeadSize];
    //memcpy(pHeader->pFlvHeader, pBuf, pHeader->nHeadSize);
    std::copy_n(pBuf,pHeader->nHeadSize,pHeader->pFlvHeader);

    return pHeader;
}

int CFlvParser::DestroyFlvHeader(FlvHeader *pHeader)
{
    if (!pHeader)
        return 0;

    delete pHeader->pFlvHeader;
    return 1;
}

/**
 * @brief 复制header + body
 * @param pHeader
 * @param pBuf
 * @param nLeftLen
 */
void CFlvParser::Tag::Init(const TagHeader *pHeader,
    const uint8_t *pBuf,const int nLeftLen)
{
    memcpy(&_header, pHeader, sizeof(TagHeader));
    // 复制标签头部信息 header
    _pTagHeader = new uint8_t[11];
    //memcpy(_pTagHeader, pBuf, 11);      // 头部
    std::copy_n(pBuf,11,_pTagHeader);
    // 复制标签 body
    _pTagData = new uint8_t[_header.nDataSize];
    //memcpy(_pTagData, pBuf + 11, _header.nDataSize);
    std::copy_n(pBuf + 11,_header.nDataSize,_pTagData);
}

CFlvParser::CVideoTag::CVideoTag(const TagHeader *pHeader,
    const uint8_t *pBuf,
    const int nLeftLen,
    CFlvParser *pParser) {
    // 初始化,把整个tag(包含头部)拷贝到自身成员变量
    Init(pHeader, pBuf, nLeftLen);

    const auto pd {_pTagData};/*tag data的起始位置*/
    _nFrameType = (pd[0] & 0xf0) >> 4;  // 帧类型
    _nCodecID = pd[0] & 0x0f;           // 视频编码类型
    // 开始解析
    if ((0x09 == _header.nType) && (7 == _nCodecID) ) {
        ParseH264Tag(pParser);
    }
}

/**
 * @brief  CAudioTag 音频Tag Data区域开始的第一个字节包含了音频数据的参数信息，
 * 从第二个字节开始为音频流数据，但第二个字节对于AAC也要判断是AAC sequence header还是AAC raw。
 * 第一个字节：SoundFormat 4bit 音频格式 0 = Linear PCM, platform endian
                        1 =ADPCM; 2 = MP3; 3 = Linear PCM, little endian
                        4 = Nellymoser 16-kHz mono ; 5 = Nellymoser 8-kHz mono
                        6 = Nellymoser;  7 = G.711 A-law logarithmic PCM
                        8 = G.711 mu-law logarithmic PCM; 9 = reserved
                        10 = AAC ; 11  Speex 14 = MP3 8-Khz
                        15 = Device-specific sound
              SoundRate 2bit 采样率 0 = 5.5-kHz; 1 = 11-kHz; 2 = 22-kHz; 3 = 44-kHz
                        对于AAC总是3。但实际上AAC是可以支持到48khz以上的频率。
              SoundSize 1bit 采样精度  0 = snd8Bit; 1 = snd16Bit
                        此参数仅适用于未压缩的格式，压缩后的格式都是将其设为1
              SoundType 1bit  0 = sndMono 单声道; 1 = sndStereo 立体声，双声道
                        对于AAC总是1
If the SoundFormat indicates AAC, the SoundType should be set to 1 (stereo) and the
SoundRate should be set to 3 (44 kHz). However, this does not mean that AAC audio in FLV
is always stereo, 44 kHz data. Instead, the Flash Player ignores these values and
extracts the channel and sample rate data is encoded in the AAC bitstream.
 * @param pHeader
 * @param pBuf
 * @param nLeftLen
 * @param pParser
 */
CFlvParser::CAudioTag::CAudioTag(TagHeader *pHeader,
    const uint8_t *pBuf,
    const int nLeftLen, CFlvParser *pParser)
{
    Init(pHeader, pBuf, nLeftLen);

    auto pd { _pTagData};
    _nSoundFormat = (pd[0] & 0xf0) >> 4;    // 音频格式
    _nSoundRate = (pd[0] & 0x0c) >> 2;      // 采样率
    _nSoundSize = (pd[0] & 0x02) >> 1;      // 采样精度
    _nSoundType = (pd[0] & 0x01);           // 是否立体声
    if (0x0a == _nSoundFormat) {               // AAC
        ParseAACTag(pParser);
    }
}

int CFlvParser::CAudioTag::ParseAACTag(CFlvParser *pParser)
{
    const auto pd {_pTagData};

    // 数据包的类型：音频配置信息，音频数据
    auto nAACPacketType { static_cast<uint32_t>(pd[1])};

    // 如果是音频配置信息
    if (!nAACPacketType){    // AAC sequence header
        // 解析配置信息
        ParseAudioSpecificConfig(pParser, pd); // 解析AudioSpecificConfig
    }else {/* 如果是音频数据  AAC RAW*/
        // 解析音频数据
        ParseRawAAC(pParser, pd);
    }

    return 1;
}

int CFlvParser::CAudioTag::ParseAudioSpecificConfig(CFlvParser *pParser,
    const uint8_t *pTagData)
{
    const auto pd { _pTagData};

    _aacProfile = (pd[2] & 0xf8) >> 3;    // 5bit AAC编码级别 /*AudioObjectType*/
    _sampleRateIndex = ( (pd[2] & 0x07) << 1 ) | (pd[3] >> 7);  // 4bit 真正的采样率索引
    _channelConfig = ( pd[3] >> 3 ) & 0x0f;                 // 4bit 通道数量

    std::cout << "----- AAC ------\n";
    std::cout << "profile : " << _aacProfile << "\n";
    std::cout << "sample rate index : " << _sampleRateIndex << "\n";
    std::cout << "channel config : " << _channelConfig << "\n";

    _pMedia = nullptr;
    _nMediaLen = 0;
    return 1;
}

int CFlvParser::CAudioTag::ParseRawAAC(CFlvParser *pParser, uint8_t *pTagData)
{
    uint64_t bits {};  // 占用8字节
    // 数据长度 跳过tag data的第一个第二字节
    const auto dataSize { _header.nDataSize - 2};   // 减去两字节的 audio tag data信息部分

    // 制作元数据
    WriteU64(bits, 12, 0xFFF);
    WriteU64(bits, 1, 0);
    WriteU64(bits, 2, 0);
    WriteU64(bits, 1, 1);
    WriteU64(bits, 2, _aacProfile - 1);
    WriteU64(bits, 4, _sampleRateIndex);
    WriteU64(bits, 1, 0);
    WriteU64(bits, 3, _channelConfig);
    WriteU64(bits, 1, 0);
    WriteU64(bits, 1, 0);
    WriteU64(bits, 1, 0);
    WriteU64(bits, 1, 0);
    WriteU64(bits, 13, 7 + dataSize);
    WriteU64(bits, 11, 0x7FF);
    WriteU64(bits, 2, 0);
    // WriteU64执行为上述的操作,最高的8bit还没有被移位到,实际是使用7个字节
    _nMediaLen = 7 + dataSize;/*ATDS head size + datasize*/
    _pMedia = new uint8_t[_nMediaLen];
    uint8_t p64[8]{};
    p64[0] = static_cast<uint8_t>(bits >> 56); // 是bits的最高8bit，实际为0
    p64[1] = static_cast<uint8_t>(bits >> 48); // 才是ADTS起始头 0xfff的高8bit
    p64[2] = static_cast<uint8_t>(bits >> 40);
    p64[3] = static_cast<uint8_t>(bits >> 32);
    p64[4] = static_cast<uint8_t>(bits >> 24);
    p64[5] = static_cast<uint8_t>(bits >> 16);
    p64[6] = static_cast<uint8_t>(bits >> 8);
    p64[7] = static_cast<uint8_t>(bits);

    //memcpy(_pMedia, p64+1, 7);  // ADTS header, p64+1是从ADTS起始头开始
    std::copy_n(p64+1,7,_pMedia);
    //memcpy(_pMedia + 7, pTagData + 2, dataSize); // AAC body
    std::copy_n(pTagData + 2,dataSize,_pMedia + 7);

    return 1;
}

CFlvParser::CMetaDataTag::CMetaDataTag(TagHeader *pHeader,const uint8_t *pBuf,
    const int nLeftLen, CFlvParser *pParser)
{
    Init(pHeader, pBuf, nLeftLen);

    const auto pd { _pTagData};
    m_amf1_type = ShowU8(pd+0);
    m_amf1_size = ShowU16(pd+1);

    if(2!= m_amf1_type){
        std::cout << "no metadata\n";
        return;
    }
    // 解析script
    if(!strncmp((const char *)"onMetaData", reinterpret_cast<const char *>(pd + 3), 10)){
        parseMeta(pParser);
    }
}

double CFlvParser::CMetaDataTag::hexStr2double(const uint8_t* hex,
                                               const uint32_t length) {
    const auto size{length * 2};
    char hexstr[size];
    std::fill_n(hexstr,length * 2,0);

    for(uint32_t i {}; i < length; i++){
        sprintf(hexstr + i * 2, "%02x", hex[i]);
    }

    double ret {};
    sscanf(hexstr, "%llx", (unsigned long long*)&ret);
    return ret;
}
int CFlvParser::CMetaDataTag::parseMeta(CFlvParser *pParser)
{
    const auto pd {_pTagData};
    //int dataSize = _header.nDataSize;

    uint32_t arrayLen {};
    uint32_t offset {13}; // Type + Value_Size + Value占用13字节

    //cout << "sizeof(string) " << sizeof(string) << "\n";
   // bool boolValue = false;
    uint32_t valueLen {};
    uint8_t u8Value {};
    if(pd[offset++] == 0x08){    // 0x8 onMetaData
        arrayLen = ShowU32(pd + offset);
        offset += 4;    //跳过 [ECMAArrayLength]占用的字节
        std::cout << "ArrayLen = " << arrayLen << "\n";
    }else{
        std::cout << "metadata format error!!!\n";
        return -1;
    }

    string strValue;

    for(uint32_t i {}; i < arrayLen; i++)
    {
       double doubleValue {};
        bool boolValue {};

        // 读取字段长度
        const auto nameLen {ShowU16(pd + offset)};
        offset += 2;            // 跳过2字节字段长度

        string name(&pd[offset],&pd[offset] + nameLen);

        offset += nameLen;      // 跳过字段名占用的长度

        const auto amfType {pd[offset++]};

        switch(amfType)    // 判别值的类型
        {
        case 0x0: //Number type, 就是double类型, 占用8字节
            doubleValue = hexStr2double(&pd[offset], 8);
            offset += 8;        // 跳过8字节
            break;

        case 0x1: //Boolean type, 占用1字节
            u8Value = ShowU8(pd+offset);
            offset += 1;        // 跳过1字节
            if(u8Value != 0x00)
                 boolValue = true;
            else
                 boolValue = false;
            break;

        case 0x2: //String type
            valueLen = ShowU16(pd + offset);
            offset += 2;    // 跳过2字节 length
            strValue = {pd + offset, pd + offset + valueLen};
            offset += valueLen; // 跳过字段的值的长度
            break;

        default:
            std::cout << "un handle amfType: " << amfType << "\n";
            break;
        }


        if(strncmp(name.c_str(), "duration", 8)	== 0)
        {
            m_duration = doubleValue;
        }
        else if(strncmp(name.c_str(), "width", 5)	== 0)
        {
            m_width = doubleValue;
        }
        else if(strncmp(name.c_str(), "height", 6) == 0)
        {
            m_height = doubleValue;
        }
        else if(strncmp(name.c_str(), "videodatarate", 13) == 0)
        {
            m_videodatarate = doubleValue;
        }
        else if(strncmp(name.c_str(), "framerate", 9) == 0)
        {
            m_framerate = doubleValue;
        }
        else if(strncmp(name.c_str(), "videocodecid", 12) == 0)
        {
            m_videocodecid = doubleValue;
        }
        else if(strncmp(name.c_str(), "audiodatarate", 13) == 0)
        {
            m_audiodatarate = doubleValue;
        }
        else if(strncmp(name.c_str(), "audiosamplerate", 15) == 0)
        {
            m_audiosamplerate = doubleValue;
        }
        else if(strncmp(name.c_str(), "audiosamplesize", 15) == 0)
        {
            m_audiosamplesize = doubleValue;
        }
        else if(strncmp(name.c_str(), "stereo", 6) == 0)
        {
            m_stereo = boolValue;
        }
        else if(strncmp(name.c_str(), "audiocodecid", 12) == 0)
        {
            m_audiocodecid = doubleValue;
        }
        else if(strncmp(name.c_str(), "major_brand", 11) == 0)
        {
            m_major_brand = strValue;
        }
        else if(strncmp(name.c_str(), "minor_version", 13) == 0)
        {
           m_minor_version = strValue;
        }
        else if(strncmp(name.c_str(), "compatible_brands", 17) == 0)
        {
            m_compatible_brands = strValue;
        }
        else if(strncmp(name.c_str(), "encoder", 7) == 0)
        {
            m_encoder = strValue;
        }
        else if(strncmp(name.c_str(), "filesize", 8) == 0)
        {
            m_filesize = doubleValue;
        }
        //delete name;
    }

    printMeta();
    return 1;
}

void CFlvParser::CMetaDataTag::printMeta()
{
    printf("\nduration: %0.2lfs, filesize: %.0lfbytes\n", m_duration, m_filesize);

    printf("width: %0.0lf, height: %0.0lf\n", m_width, m_height);
    printf("videodatarate: %0.2lfkbps, framerate: %0.0lffps\n", m_videodatarate, m_framerate);
    printf("videocodecid: %0.0lf\n", m_videocodecid);

    printf("audiodatarate: %0.2lfkbps, audiosamplerate: %0.0lfKhz\n",
           m_audiodatarate, m_audiosamplerate);
    printf("audiosamplesize: %0.0lfbit, stereo: %d\n", m_audiosamplesize, m_stereo);
    printf("audiocodecid: %0.0lf\n", m_audiocodecid);

    printf("major_brand: %s, minor_version: %s\n", m_major_brand.c_str(), m_minor_version.c_str());
    printf("compatible_brands: %s, encoder: %s\n\n", m_compatible_brands.c_str(), m_encoder.c_str());
}

CFlvParser::Tag *CFlvParser::CreateTag(const uint8_t *pBuf,const int nLeftLen)
{
    // 开始解析标签头部
    TagHeader header;
    header.nType = static_cast<int>(ShowU8(pBuf+0));  // 类型
    header.nDataSize = static_cast<int>(ShowU24(pBuf + 1));   // 标签body的长度
    header.nTimeStamp = static_cast<int>(ShowU24(pBuf + 4));  // 时间戳 低24bit
    header.nTSEx = static_cast<int>(ShowU8(pBuf + 7));        // 时间戳的扩展字段, 高8bit
    header.nStreamID = static_cast<int>(ShowU24(pBuf + 8));   // 流的id
    header.nTotalTS = static_cast<uint32_t>((header.nTSEx << 24)) + header.nTimeStamp;
    // 标签头部解析结束

//    cout << "total TS : " << header.nTotalTS << endl;
//    cout << "nLeftLen : " << nLeftLen << " , nDataSize : " << header.nDataSize << endl;

    if ((header.nDataSize + 11) > nLeftLen){
        return nullptr;
    }

    Tag *pTag{};
    switch (header.nType) {
    case 0x09:  // 视频类型的Tag
        pTag = new CVideoTag(&header, pBuf, nLeftLen, this);
        break;
    case 0x08:  // 音频类型的Tag
        pTag = new CAudioTag(&header, pBuf, nLeftLen, this);
        break;
    case 0x12:  // script Tag
        pTag = new CMetaDataTag(&header, pBuf, nLeftLen, this);
        break;
    default:    // script类型的Tag
        pTag = new Tag();
        pTag->Init(&header, pBuf, nLeftLen);
        break;
    }

    return pTag;
}

int CFlvParser::DestroyTag(const Tag * const pTag)
{
    delete []pTag->_pMedia;
    delete []pTag->_pTagData;
    delete []pTag->_pTagHeader;
    return 1;
}

int CFlvParser::CVideoTag::ParseH264Tag(CFlvParser *pParser)
{
    const auto pd { _pTagData};
    /*
    ** 数据包的类型
    ** 视频数据被压缩之后被打包成数据包在网上传输
    ** 有两种类型的数据包：视频信息包（sps、pps等）和视频数据包（视频的压缩数据）
    */
    const auto nAVCPacketType {static_cast<int>(pd[1])};
   // int nCompositionTime = CFlvParser::ShowU24(pd + 2);
   // AVCPacketType 0:Configuration 1:AVC NALU
    if (nAVCPacketType) { // AVC NALU
        // 如果是视频数据
        ParseNalu(pParser, pd);
    }else{  // AVC sequence header
        // 如果是视频配置信息
        ParseH264Configuration(pParser, pd);
    }
    return 1;
}
/**
 * @brief
 * []里面的数值是位置偏移量(下标)
AVCDecoderConfigurationRecord {
    uint32_t(8bit) configurationVersion = 1;   [0]
    uint32_t(8bit) AVCProfileIndication;       [1]
    uint32_t(8bit) profile_compatibility;      [2]
    uint32_t(8bit) AVCLevelIndication;         [3]
    bit(6bit) reserved = ‘111111’b;            [4]
    uint32_t(2bit) lengthSizeMinusOne;         [4] 计算方法是 1 + (lengthSizeMinusOne & 3),实际计算结果一直是4
    bit(3bit) reserved = ‘111’b;               [5]
    uint32_t(5bit) numOfSequenceParameterSets; [5] SPS 的个数,计算方法是 (numOfSequenceParameterSets & 0x1F),实际计算结果一直为1
    for (i=0; i< numOfSequenceParameterSets; i++) {
        uint32_t(16bit) sequenceParameterSetLength ;   [6,7]
        bit(8bit * sequenceParameterSetLength) sequenceParameterSetNALUnit;  25个字节    [8~32]
    }
    uint32_t(8bit) numOfPictureParameterSets;      PPS 的个数，一直为1    [33]
    for (i=0; i< numOfPictureParameterSets; i++) {
        uint32_t(16bit) pictureParameterSetLength;     [34,35]
        bit(8bit * pictureParameterSetLength) pictureParameterSetNALUnit;    [36~40]
    }
}

_nNalUnitLength 这个变量告诉我们用几个字节来存储NALU的长度,如果NALULengthSizeMinusOne是0,
那么每个NALU使用一个字节的前缀来指定长度,那么每个NALU包的最大长度是255字节,
这个明显太小了,使用2个字节的前缀来指定长度,那么每个NALU包的最大长度是64K字节,
也不一定够,一般分辨率达到 1280*720 的图像编码出的I帧,可能大于64K;3字节是比较完美的,
但是因为一些原因(例如对齐)没有被广泛支持;因此4字节长度的前缀是目前使用最多的方式

 * @param pParser
 * @param pTagData
 * @return
 */

int CFlvParser::CVideoTag::ParseH264Configuration(CFlvParser *pParser,
    const uint8_t *pTagData)
{
    const auto pd {pTagData};
    // 跨过 Tag Data的VIDEODATA(frametype + codecid)(1字节)
    // AVCVIDEOPACKET(AVCPacketType(1字节) 和CompositionTime(3字节) 4字节)
    // 总共跨过5个字节

    // NalUnit长度表示占用的字节
    pParser->_nNalUnitLength = (pd[9] & 0x03) + 1;  // lengthSizeMinusOne posloca = 9 = 5 + 4

    // sps（序列参数集）的长度
    const auto sps_size { CFlvParser::ShowU16(pd + 11)};        // sequenceParameterSetLength 11 = 5 + 6
    // pps（图像参数集）的长度
    const auto pps_size {CFlvParser::ShowU16(pd + 11 + (2 + sps_size) + 1)};   // pictureParameterSetLength

    // 元数据的长度
    _nMediaLen = 4 + sps_size + 4 + pps_size;   // 添加start code
    _pMedia = new uint8_t[_nMediaLen];

    // 保存元数据
    //memcpy(_pMedia, &nH264StartCode, 4);
    std::copy_n(reinterpret_cast<const uint8_t*>(&nH264StartCode),4,_pMedia);

    //memcpy(_pMedia + 4, pd + 11 + 2, sps_size);
    std::copy_n(pd + 11 + 2,sps_size,_pMedia + 4);

    //memcpy(_pMedia + 4 + sps_size, &nH264StartCode, 4);
    std::copy_n(reinterpret_cast<const uint8_t*>(&nH264StartCode),4,_pMedia + 4 + sps_size);

    //memcpy(_pMedia + 4 + sps_size + 4, pd + 11 + 2 + sps_size + 2 + 1, pps_size);
    std::copy_n(pd + 11 + 2 + sps_size + 2 + 1,pps_size,_pMedia + 4 + sps_size + 4);

    return 1;
}

int CFlvParser::CVideoTag::ParseNalu(CFlvParser *pParser,const uint8_t *pTagData)
{
    const auto pd { pTagData};
    //int nOffset {};

    _pMedia = new uint8_t[_header.nDataSize + 10];
    _nMediaLen = 0;
    // 跨过 Tag Data的VIDEODATA(Frame_Type + CodecID)(1字节)
    // AVCVIDEOPACKET(AVCPacketType和CompositionTime 4字节)
    int nOffset {5}; // 总共跨过5个字节 132 - 5 = 127 = _nNalUnitLength(4字节)  + NALU(123字节)
    //                                           startcode(4字节)  + NALU(123字节) = 127
    for(;;) {
        // 如果解析完了一个Tag，那么就跳出循环
        if (nOffset >= _header.nDataSize){
            break;
        }
        // 计算NALU（视频数据被包装成NALU在网上传输）的长度,
        // 一个tag可能包含多个nalu,所以每个nalu前面有NalUnitLength字节表示每个nalu的长度
        int nNaluLen{};

        switch (pParser->_nNalUnitLength) {/*_nNalUnitLength一般都是4,这里判断只是为了防止有其他情况出现*/
            /* pParser->_nNalUnitLength决定存放NALU length需要用多少个字节,默认是4个字节 */
        case 4:
            nNaluLen = static_cast<int>(CFlvParser::ShowU32(pd + nOffset));
            break;
        case 3:
            nNaluLen = static_cast<int>(CFlvParser::ShowU24(pd + nOffset));
            break;
        case 2:
            nNaluLen = static_cast<int>(CFlvParser::ShowU16(pd + nOffset));
            break;
        default:
            nNaluLen = static_cast<int>(CFlvParser::ShowU8(pd + nOffset));
            break;
        }
        // 获取NALU的起始码
        //memcpy(_pMedia + _nMediaLen, &nH264StartCode, 4);
        std::copy_n(reinterpret_cast<const uint8_t*>(&nH264StartCode),4,_pMedia + _nMediaLen);
        // 复制NALU的数据
        //memcpy(_pMedia + _nMediaLen + 4, pd + nOffset + pParser->_nNalUnitLength, nNaluLen);
        std::copy_n(pd + nOffset + pParser->_nNalUnitLength,nNaluLen,_pMedia + _nMediaLen + 4);

        /* pParser->_nNalUnitLength决定存放NALU length需要用多少个字节,默认是4个字节 */

        // 解析NALU
//        pParser->_vjj->Process(_pMedia+_nMediaLen, 4+nNaluLen, _header.nTotalTS);
        _nMediaLen += (4 + nNaluLen);
        /*4代表前面start_code(4字节) + 真正数据的大小(已除去了payload的前4个字节)*/

        nOffset += (pParser->_nNalUnitLength + nNaluLen); /* 4 + 每个NALU data前4个字节就是当前NALU的长度*/
        /*nOffset实际偏移一个NALU payload的长度(4字节 + 真正数据长度)*/

        /*这里_pMedia保存的是每一个没有NALU payload长度的数据,就是真正数据的前面4个字节*/
    }

    return 1;
}
