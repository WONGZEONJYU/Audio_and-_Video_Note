//
// Created by Administrator on 2024/5/6.
//

#ifndef INC_18_PICTURE_WATERMARK_WATERMARK_DEMO_HPP
#define INC_18_PICTURE_WATERMARK_WATERMARK_DEMO_HPP

extern "C"{
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
#include <libavcodec/avcodec.h>
};



#include "ShareAVFrame.hpp"
#include "ShareAVPacket.hpp"

class Watermark_Demo final {

    static ShareAVFrame_sp_type get_frame_from_jpeg_file(const std::string &) noexcept(false);
    explicit Watermark_Demo(const char **) noexcept(true);
    void Construct() noexcept(false);
    void DeConstruct() noexcept(true);
    void get_picture() noexcept(false);
    void init_filters() noexcept(false);
    void main_picture_mix_logo() noexcept(false);
    void save_picture() noexcept(false);

public:
    using Watermark_Demo_sp_type = std::shared_ptr<Watermark_Demo>;
    Watermark_Demo(const Watermark_Demo&) = delete;
    Watermark_Demo& operator=(const Watermark_Demo&) = delete;
    static Watermark_Demo_sp_type create(const char **) noexcept(false);
    ~Watermark_Demo();
    void exec() noexcept(false);

private:
    const char **m_argv{};
    AVFilterGraph *m_avFilterGraph{};
    ShareAVFrame_sp_type m_main_frame,m_logo_frame,m_out_frame;
    AVFilterContext *m_main_src_buffer_filter_ctx{},
    *m_logo_src_buffer_filter_ctx{},
    *m_sink_buffer_filter_ctx{};
};

using Watermark_Demo_sp_type = Watermark_Demo::Watermark_Demo_sp_type;

Watermark_Demo_sp_type new_Watermark_Demo(const char **) noexcept(false);


#endif //INC_18_PICTURE_WATERMARK_WATERMARK_DEMO_HPP
