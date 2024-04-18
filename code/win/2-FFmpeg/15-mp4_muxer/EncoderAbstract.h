//
// Created by Administrator on 2024/4/18.
//

#ifndef INC_15_MP4_MUXER_ENCODERABSTRACT_H
#define INC_15_MP4_MUXER_ENCODERABSTRACT_H

#include <vector>
#include <memory>

struct AVRational;
struct AVFrame;
struct AVPacket;

class EncoderAbstract {

public:
    using EncoderAbstract_sp_type = std::shared_ptr<EncoderAbstract>;
    EncoderAbstract(const EncoderAbstract&) = delete;
    EncoderAbstract& operator=(const EncoderAbstract&) = delete;

    virtual void encode(AVFrame *frame,const int &stream_index,const long long &pts,
                       const AVRational& time_base,std::vector<AVPacket*>& packets) const noexcept(false) = 0;
    [[nodiscard]] virtual AVRational time_base() const noexcept(true) = 0;

protected:
    explicit EncoderAbstract() = default;
    virtual ~EncoderAbstract() = default;

};

#endif
