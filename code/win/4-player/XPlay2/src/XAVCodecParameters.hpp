//
// Created by Administrator on 2024/7/30.
//

#ifndef XPLAY2_XAVCODECPARAMETERS_HPP
#define XPLAY2_XAVCODECPARAMETERS_HPP

extern "C"{
#include <libavcodec/codec_par.h>
}

#include <memory>
#include <string>
#include "XHelper.hpp"

struct AVCodecContext;
class XAVCodecParameters;
using XAVCodecParameters_sptr = std::shared_ptr<XAVCodecParameters>;

class XAVCodecParameters final : AVCodecParameters{
    static void Reset(AVCodecParameters*) noexcept(true);
    void Move(AVCodecParameters*) noexcept(true);
public:
    XAVCodecParameters();
    explicit XAVCodecParameters(const AVCodecContext *) noexcept(false);
    explicit XAVCodecParameters(const AVCodecParameters *) noexcept(false);
    XAVCodecParameters(const XAVCodecParameters &) noexcept(false);
    XAVCodecParameters(XAVCodecParameters &&) noexcept(true);
    XAVCodecParameters& operator=(const XAVCodecParameters &) noexcept(false);
    XAVCodecParameters& operator=(XAVCodecParameters &&) noexcept(true);

    void from_AVFormatContext(const AVCodecParameters *)  noexcept(false);
    void from_context(const AVCodecContext *)  noexcept(false);
    void to_context(AVCodecContext *) const noexcept(false);
    [[nodiscard]] auto MediaType() const noexcept(true){
        return codec_type;
    }

    [[nodiscard]] auto Codec_id() const noexcept(true){
        return codec_id;
    }

    [[nodiscard]] std::string Codec_name() const noexcept(true);

    [[nodiscard]] auto Ch_layout() const noexcept(true){
        return &ch_layout;
    }

    [[nodiscard]] auto Sample_Format() const noexcept(true){
        return format;
    }

    [[nodiscard]] auto Sample_rate() const noexcept(true){
        return sample_rate;
    }

    [[nodiscard]] auto Width() const noexcept(true){
        return width;
    }

    [[nodiscard]] auto Height() const noexcept(true){
        return height;
    }

    ~XAVCodecParameters();
};

XAVCodecParameters_sptr new_XAVCodecParameters() noexcept(false);
XAVCodecParameters_sptr new_XAVCodecParameters(const AVCodecContext *) noexcept(false);
XAVCodecParameters_sptr new_XAVCodecParameters(const AVCodecParameters *) noexcept(false);

#endif
