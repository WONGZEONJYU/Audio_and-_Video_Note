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
#include "xhelper.hpp"

struct AVCodecContext;
class XAVCodecParameters;
using XAVCodecParameters_sptr = typename std::shared_ptr<XAVCodecParameters>;

class XAVCodecParameters final : AVCodecParameters {
    static void Reset(AVCodecParameters *) noexcept(true);
    void Move(XAVCodecParameters *) noexcept(true);
public:
    XAVCodecParameters();
    explicit XAVCodecParameters(const AVCodecContext *,const AVRational & = {1,1}) noexcept(false);
    explicit XAVCodecParameters(const AVCodecParameters *,const AVRational & = {1,1}) noexcept(false);
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

    [[nodiscard]] int Sample_Format_Size() const noexcept(true);

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

    void set_time_base(const AVRational &tb) noexcept(true){
        m_time_base = tb;
    }

    [[nodiscard]] auto time_base() const noexcept(true){return m_time_base;}

private:
    AVRational m_time_base{1,1};
};

XAVCodecParameters_sptr new_XAVCodecParameters() noexcept(false);
XAVCodecParameters_sptr new_XAVCodecParameters(const AVCodecContext *,const AVRational & = {1,1}) noexcept(false);
XAVCodecParameters_sptr new_XAVCodecParameters(const AVCodecParameters *,const AVRational & = {1,1}) noexcept(false);

#endif