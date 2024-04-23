//
// Created by Administrator on 2024/4/18.
//

#ifndef INC_15_MP4_MUXER_VIDEOOUTPUTSTREAM_H
#define INC_15_MP4_MUXER_VIDEOOUTPUTSTREAM_H

#include "OutputStreamAbstract.h"

struct Video_Encoder_params;
class Muxer;

class VideoOutputStream final : public OutputStreamAbstract{

    explicit VideoOutputStream()  = default;
    void Construct(const std::shared_ptr<Muxer>&,
            const Video_Encoder_params&) noexcept(false);
public:
    using VideoOutputStream_sp_type = std::shared_ptr<VideoOutputStream>;
    static VideoOutputStream_sp_type create(const std::shared_ptr<Muxer>&,
                                            const Video_Encoder_params&);
};

using VideoOutputStream_sp_type = typename VideoOutputStream::VideoOutputStream_sp_type;

VideoOutputStream_sp_type new_VideoOutputStream(const std::shared_ptr<Muxer>&,
                                                const Video_Encoder_params&) noexcept(false);
#endif
