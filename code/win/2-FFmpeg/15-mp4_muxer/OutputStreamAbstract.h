//
// Created by Administrator on 2024/4/17.
//

#ifndef INC_15_MP4_MUXER_OUTPUTSTREAMABSTRACT_H
#define INC_15_MP4_MUXER_OUTPUTSTREAMABSTRACT_H

#include <memory>

extern "C"{
#include <libavutil/rational.h>
}

class OutputStreamAbstract {

public:
    OutputStreamAbstract(const OutputStreamAbstract&) = delete;
    OutputStreamAbstract& operator=(const OutputStreamAbstract&) = delete;

    using OutputStreamAbstract_sp_type = std::shared_ptr<OutputStreamAbstract>;
    [[nodiscard]] virtual int Stream_index() const noexcept = 0;
    [[nodiscard]] virtual AVRational Stream_time_base() const noexcept = 0;

protected:
    explicit OutputStreamAbstract() = default;
    virtual ~OutputStreamAbstract() = default;
};

#endif
