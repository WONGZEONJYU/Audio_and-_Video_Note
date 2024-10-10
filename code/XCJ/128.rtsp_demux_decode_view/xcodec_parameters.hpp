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

using XCodecParameters_sp = std::shared_ptr<XCodecParameters>;

class XCodecParameters final : AVCodecParameters {
    static void Reset(AVCodecParameters *) noexcept(true);
    void Move(XCodecParameters *) noexcept(true);
public:
    XCodecParameters();
    explicit XCodecParameters(const AVCodecContext *,const AVRational & = {1,1}) noexcept(false);
    explicit XCodecParameters(const AVCodecParameters *,const AVRational & = {1,1}) noexcept(false);
    XCodecParameters(const XCodecParameters &) noexcept(false);
    XCodecParameters(XCodecParameters &&) noexcept(true);
    XCodecParameters& operator=(const XCodecParameters &) noexcept(false);
    XCodecParameters& operator=(XCodecParameters &&) noexcept(true);

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

    ~XCodecParameters();

    void set_time_base(const AVRational &tb) noexcept(true){
        m_time_base = tb;
    }

    [[nodiscard]] auto time_base() const noexcept(true){return m_time_base;}

private:
    AVRational m_time_base{1,1};
};

XCodecParameters_sp new_XCodecParameters();
XCodecParameters_sp new_XCodecParameters(const AVCodecParameters *src,const AVRational &tb = {1,1});
XCodecParameters_sp new_XCodecParameters(const AVCodecContext *src,const AVRational &tb = {1,1});

#endif

