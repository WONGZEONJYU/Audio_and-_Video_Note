//
// Created by Administrator on 2024/4/18.
//

#ifndef INC_15_MP4_MUXER_VIDEOOUTPUTSTREAM_H
#define INC_15_MP4_MUXER_VIDEOOUTPUTSTREAM_H

#include <memory>
#include "OutputStreamAbstract.h"

#include "EncoderAbstract.h"

struct AVStream;

class VideoOutputStream: public OutputStreamAbstract{

    void Construct() noexcept(false);
    explicit VideoOutputStream()  = default;

public:
    using VideoOutputStream_sp_type = std::shared_ptr<VideoOutputStream>;
    static VideoOutputStream_sp_type create();

private:

};

#endif
