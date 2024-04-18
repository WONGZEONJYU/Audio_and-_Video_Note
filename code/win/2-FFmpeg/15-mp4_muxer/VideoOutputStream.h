//
// Created by Administrator on 2024/4/18.
//

#ifndef INC_15_MP4_MUXER_VIDEOOUTPUTSTREAM_H
#define INC_15_MP4_MUXER_VIDEOOUTPUTSTREAM_H

#include <memory>
#include "OutputStreamAbstract.h"

struct AVStream;
struct AVFrame;

class VideoOutputStream: public OutputStreamAbstract{

    void Construct() noexcept(false);
    explicit VideoOutputStream()  = default;

public:
    using VideoOutputStream_sp_type = std::shared_ptr<VideoOutputStream>;
    static VideoOutputStream_sp_type create();
    [[nodiscard]] int Stream_index() const noexcept override;
    [[nodiscard]] AVRational Stream_time_base() const noexcept override;
private:
    AVStream *m_stream{};
    AVFrame * m_frame{};
    int m_stream_index{};
    AVRational m_time_base{};
};

#endif
