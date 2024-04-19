//
// Created by Administrator on 2024/4/18.
//

#ifndef INC_15_MP4_MUXER_VIDEOOUTPUTSTREAM_H
#define INC_15_MP4_MUXER_VIDEOOUTPUTSTREAM_H

#include "OutputStreamAbstract.h"
#include "VideoEncoder.h"

struct AVStream;

class Muxer;

class VideoOutputStream: public OutputStreamAbstract{

    explicit VideoOutputStream()  = default;
    void Construct(const Muxer*) noexcept(false);
public:
    using VideoOutputStream_sp_type = std::shared_ptr<VideoOutputStream>;
    static VideoOutputStream_sp_type create(const Muxer *);

private:
    VideoEncoder_sp_type video_encoder;
};

using VideoOutputStream_sp_type = typename VideoOutputStream::VideoOutputStream_sp_type;

#endif
