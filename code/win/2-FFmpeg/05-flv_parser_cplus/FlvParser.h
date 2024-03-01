#ifndef FLVPARSER_H
#define FLVPARSER_H

#include <cstdint>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <variant>
#include <unordered_map>
#include "Videojj.h"

class CFlvParser
{
public:
    CFlvParser();
    virtual ~CFlvParser();

    int Parse(const uint8_t *pBuf, int nBufSize, int &nUsedLen);
    int PrintInfo();
    [[nodiscard]]int DumpH264(const std::string &) const;
    int DumpAAC(const std::string &);
    [[nodiscard]]int DumpFlv(const std::string &) const;

private:
    // FLV头
    struct FlvHeader {
        int nVersion; // 版本
        int bHaveVideo; // 是否包含视频
        int bHaveAudio; // 是否包含音频
        int nHeadSize;  // FLV头部长度
        /*
        ** 指向存放FLV头部的buffer
        ** 上面的三个成员指明了FLV头部的信息，是从FLV的头部中“翻译”得到的，
        ** 真实的FLV头部是一个二进制比特串，放在一个buffer中，由pFlvHeader成员指明
        */
        uint8_t *pFlvHeader;
    };

    // Tag头部
    struct TagHeader {
        int nType{};      // 类型
        int nDataSize{};  // 标签body的大小
        int nTimeStamp{}; // 时间戳
        int nTSEx{};      // 时间戳的扩展字节
        int nStreamID{};  // 流的ID，总是0
        uint32_t nTotalTS{};  // 完整的时间戳nTimeStamp和nTSEx拼装
    };

    struct Tag{
        void Init(const TagHeader *pHeader,const uint8_t *pBuf, int nLeftLen);
        TagHeader _header{};
        uint8_t *_pTagHeader{};   // 指向标签头部
        uint8_t *_pTagData{};     // 指向标签body,原始的tag data数据
        uint8_t *_pMedia{};       // 指向标签的元数据,改造后的数据,添加start_code与去除每个NALU payload前4个字节(plyload的真正数据的大小)的数据
        int _nMediaLen{};         // 数据长度
    };

    struct CVideoTag : Tag {
        /**
         * @brief CVideoTag
         * @param pHeader
         * @param pBuf 整个tag的起始地址
         * @param nLeftLen
         * @param pParser
         */
        CVideoTag(const TagHeader *pHeader,const uint8_t *pBuf,
            int nLeftLen, CFlvParser *pParser);

        int _nFrameType;    // 帧类型
        int _nCodecID;      // 视频编解码类型
        int ParseH264Tag(CFlvParser *pParser);
        int ParseH264Configuration(CFlvParser *pParser,const uint8_t *pTagData);
        int ParseNalu(const CFlvParser *pParser,const uint8_t *pTagData);
    };

    struct CAudioTag : Tag
    {
        CAudioTag(const TagHeader *pHeader,const uint8_t *pBuf, int nLeftLen, CFlvParser *pParser);

        int _nSoundFormat;  // 音频编码类型
        int _nSoundRate;    // 采样率
        int _nSoundSize;    // 精度
        int _nSoundType;    // 类型

        // aac
        static int _aacProfile;     // 对应AAC profile
        static int _sampleRateIndex;    // 采样率索引
        static int _channelConfig;      // 通道设置

        int ParseAACTag(CFlvParser *pParser);
        int ParseAudioSpecificConfig(CFlvParser *pParser,const uint8_t *pTagData);
        int ParseRawAAC(CFlvParser *pParser, uint8_t *pTagData);
    };

    struct  CMetaDataTag : Tag
    {

        CMetaDataTag(const TagHeader *pHeader,const uint8_t *pBuf, int nLeftLen, CFlvParser *pParser);

        static double hexStr2double(const unsigned char* hex, unsigned int length);
        int parseMeta(CFlvParser *pParser);
        void printMeta() const;

        uint8_t m_amf1_type{};
        uint32_t m_amf1_size{};
        uint8_t m_amf2_type{};
        unsigned char *m_meta{};
        unsigned int m_length{};

        double m_duration{};
        double m_width{};
        double m_height{};
        double m_videodatarate{};
        double m_framerate{};
        double m_videocodecid{};

        double m_audiodatarate{};
        double m_audiosamplerate{};
        double m_audiosamplesize{};
        bool m_stereo{};
        double m_audiocodecid{};

        std::string m_major_brand;
        std::string m_minor_version;
        std::string m_compatible_brands;
        std::string m_encoder;
        double m_filesize{};

        using Metadata_Type = std::variant<double,bool,std::string>;
        std::unordered_map<std::string,Metadata_Type> Metadata;

    };

    struct FlvStat{
        int nMetaNum, nVideoNum, nAudioNum,nMaxTimeStamp,nLengthSize;
        FlvStat() : nMetaNum(0), nVideoNum(0), nAudioNum(0), nMaxTimeStamp(0), nLengthSize(0){}
    };

    static inline uint32_t ShowU32(const uint8_t *pBuf) { return (pBuf[0] << 24) | (pBuf[1] << 16) | (pBuf[2] << 8) | pBuf[3]; }
    static inline uint32_t ShowU24(const uint8_t *pBuf) { return (pBuf[0] << 16) | (pBuf[1] << 8) | (pBuf[2]); }
    static inline uint32_t ShowU16(const uint8_t *pBuf) { return (pBuf[0] << 8) | (pBuf[1]); }
    static inline uint32_t ShowU8(const uint8_t *pBuf) { return (pBuf[0]); }

    static inline void WriteU64(uint64_t & x, const int length,const int value){
        const auto mask {0xFFFFFFFFFFFFFFFF >> (64 - length)};
        x = (x << length) | (static_cast<uint64_t>(value) & mask);
    }

    static inline uint32_t WriteU32(const uint32_t n){
        uint32_t nn {};
        const auto p { reinterpret_cast<const uint8_t *>(&n) };
        const auto pp { reinterpret_cast<uint8_t *>(&nn) };
        pp[0] = p[3];
        pp[1] = p[2];
        pp[2] = p[1];
        pp[3] = p[0];
        return nn;
    }

    friend struct Tag;

    static FlvHeader* CreateFlvHeader(const uint8_t *pBuf);
    static int DestroyFlvHeader(const FlvHeader *pHeader) ;
    Tag *CreateTag(const uint8_t *pBuf, int nLeftLen);
    static int DestroyTag(const Tag *pTag);
    int Stat();
    int StatVideo(const Tag *pTag) ;
    //int IsUserDataTag(Tag *pTag);

    //CVideojj *_vjj{};
    std::shared_ptr<CVideojj> _vjj;
    std::vector<Tag *> _vpTag;
    FlvHeader* _pFlvHeader{};
    FlvStat _sStat{};
    // H.264
    int _nNalUnitLength{};
};

#endif // FLVPARSER_H
