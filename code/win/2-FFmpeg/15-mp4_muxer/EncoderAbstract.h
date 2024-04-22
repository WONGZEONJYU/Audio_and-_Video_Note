//
// Created by Administrator on 2024/4/18.
//

#ifndef INC_15_MP4_MUXER_ENCODERABSTRACT_H
#define INC_15_MP4_MUXER_ENCODERABSTRACT_H

#include <vector>
#include <memory>

extern "C"{
#include <libavcodec/avcodec.h>
};

#include "ShareAVPacket.hpp"
#include "ShareAVFrame.hpp"

using vector_type = typename std::pmr::vector<ShareAVPacket_sp_type>;

class EncoderAbstract {

public:
    using EncoderAbstract_sp_type = std::shared_ptr<EncoderAbstract>;
    EncoderAbstract(const EncoderAbstract&) = delete;
    EncoderAbstract& operator=(const EncoderAbstract&) = delete;

    void encode(const ShareAVFrame_sp_type &,const int &stream_index,const long long &pts,
                       const AVRational& time_base,vector_type& ) const noexcept(false);

    [[nodiscard]] AVRational time_base() const noexcept(true) {
        return m_codec_ctx->time_base;
    }

    int parameters_from_context(AVCodecParameters *);

protected:
    AVCodecContext *m_codec_ctx{};
    explicit EncoderAbstract() = default;
    virtual ~EncoderAbstract();
};

using EncoderAbstract_sp_type = typename EncoderAbstract::EncoderAbstract_sp_type;

#endif
