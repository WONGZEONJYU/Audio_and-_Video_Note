//
// Created by Administrator on 2024/4/17.
//

#ifndef INC_15_MP4_MUXER_OUTPUTSTREAMABSTRACT_H
#define INC_15_MP4_MUXER_OUTPUTSTREAMABSTRACT_H

extern "C"{
#include <libavutil/rational.h>
#include <libavformat/avformat.h>
}

#include "EncoderAbstract.h"

class OutputStreamAbstract {

public:
    OutputStreamAbstract(const OutputStreamAbstract&) = delete;
    OutputStreamAbstract& operator=(const OutputStreamAbstract&) = delete;

    using OutputStreamAbstract_sp_type = std::shared_ptr<OutputStreamAbstract>;

    [[nodiscard]] int Stream_index() const noexcept(true) {
        return m_stream->index;
    }

    [[nodiscard]] AVRational Stream_time_base() const noexcept(true){
        return m_stream->time_base;
    };

    virtual void encoder(const ShareAVFrame_sp_type &,const long long &pts,
                 const AVRational& time_base,vector_type& ) const noexcept(false) = 0;

protected:
    explicit OutputStreamAbstract() = default;
    virtual ~OutputStreamAbstract() = default;

    AVStream *m_stream{};
    EncoderAbstract_sp_type m_encoder;
};

using OutputStreamAbstract_sp_type = typename OutputStreamAbstract::OutputStreamAbstract_sp_type ;

#endif
