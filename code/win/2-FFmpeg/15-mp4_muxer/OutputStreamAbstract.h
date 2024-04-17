//
// Created by Administrator on 2024/4/17.
//

#ifndef INC_15_MP4_MUXER_OUTPUTSTREAMABSTRACT_H
#define INC_15_MP4_MUXER_OUTPUTSTREAMABSTRACT_H

struct AVStream;
struct AVCodecContext;
struct AVFormatContext;

class OutputStreamAbstract {

public:
    OutputStreamAbstract(const OutputStreamAbstract&) = delete;
    OutputStreamAbstract& operator=(const OutputStreamAbstract&) = delete;
protected:
    explicit OutputStreamAbstract(AVFormatContext&);
    virtual ~OutputStreamAbstract() = default;
    AVCodecContext *m_codec_ctx{};
    AVStream *m_stream{};
    int m_stream_index{};
};

#endif //INC_15_MP4_MUXER_OUTPUTSTREAMABSTRACT_H
