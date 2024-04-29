//
// Created by Administrator on 2024/4/29.
//

#ifndef INC_16_VIDEO_FILTER_AVFILTER_DEMO_HPP
#define INC_16_VIDEO_FILTER_AVFILTER_DEMO_HPP

extern "C" {
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
};

#include <memory>
#include <fstream>
#include <memory_resource>
#include "ShareAVFrame.hpp"

class AVFilter_Demo final {
    static inline constexpr auto width{768};
    static inline constexpr auto height{320};
    static inline constexpr auto resolution{width * height};
    static inline constexpr auto YUV_FMT{AV_PIX_FMT_YUV420P};

    explicit AVFilter_Demo(const std::string &,const std::string &) noexcept(true);
    void Construct() noexcept(false);
    void DeConstruct() noexcept(false);
    void init_source_filter() noexcept(false);
    void init_out_filter() noexcept(false);
    void init_split_filter() noexcept(false);
    void init_crop_filter() noexcept(false);
    void init_vflip_filter() noexcept(false);
    void init_overlay_filter() noexcept(false);
    void init_io_frame() noexcept(false);

    void link() noexcept(false);
    void check_filter_graph() noexcept(false);

public:
    using AVFilter_Demo_sp_type = std::shared_ptr<AVFilter_Demo>;
    AVFilter_Demo(const AVFilter_Demo&) = delete;
    AVFilter_Demo& operator=(const AVFilter_Demo&) = delete;
    static AVFilter_Demo_sp_type create(const std::string &,const std::string &) noexcept(false);
    ~AVFilter_Demo();
    void exec() noexcept(false);
private:
    std::pmr::unsynchronized_pool_resource m_mem_pool;
    std::ifstream m_in_yuv_file;
    std::ofstream m_out_yuv_file;
    const size_t m_read_size{};
    uint8_t *m_read_buffer{};
    AVFilterGraph *m_avFilterGraph{};
    AVFilterContext *m_bufferSrc_ctx{},
                    *m_Buffer_Sink_ctx{},
                    *m_splitFilter_ctx{},
                    *m_cropFilter_ctx{},
                    *m_vflipFilter_ctx{},
                    *m_overlayFilter_ctx{};
    ShareAVFrame_sp_type m_in_frame,m_out_frame;
    size_t frame_count{};
};

using AVFilter_Demo_sp_type = AVFilter_Demo::AVFilter_Demo_sp_type;

AVFilter_Demo_sp_type new_AVFilter_Demo(const std::string &,const std::string &) noexcept(false);

#endif //INC_16_VIDEO_FILTER_AVFILTER_DEMO_HPP
