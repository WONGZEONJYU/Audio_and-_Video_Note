//
// Created by Administrator on 2024/4/29.
//

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

#include <sstream>
#include <algorithm>
#include "AVHelper.h"
#include "AVFilter_Demo.hpp"

AVFilter_Demo::AVFilter_Demo(const std::string &in,const std::string &out) noexcept(true):
m_in_yuv_file(in,std::ios::binary),
m_out_yuv_file(out,std::ios::binary),
m_read_size(av_image_get_buffer_size(YUV_FMT,width,height,1)),
m_read_buffer(static_cast<decltype(m_read_buffer)>(m_mem_pool.allocate(m_read_size))),
m_avFilterGraph(avfilter_graph_alloc()){}

void AVFilter_Demo::Construct() noexcept(false)
{
    if (!m_in_yuv_file){
        throw std::runtime_error("open m_in_yuv_file failed\n");
    }

    if (!m_out_yuv_file){
        throw std::runtime_error("open m_out_yuv_file failed\n");
    }

    if (!m_avFilterGraph){
        throw std::runtime_error("avfilter_graph_alloc failed\n");
    }

    if (!m_read_buffer){

    }

    init_source_filter();
    init_out_filter();
    init_split_filter();
    init_crop_filter();
    init_vflip_filter();
    init_overlay_filter();
    init_io_frame();
    link();
    check_filter_graph();
    AVHelper::avfilter_graph_dump(m_avFilterGraph,"graphFile.txt");
}

void AVFilter_Demo::init_source_filter() noexcept(false)
{
    auto bufferSrc{avfilter_get_by_name("buffer")};
    if (!bufferSrc){
        throw std::runtime_error("buffer_src not found\n");
    }

    std::stringstream args;
    args << "video_size=" << width << "x" << height << ":";
    args << "pix_fmt=" << AV_PIX_FMT_YUV420P << ":";
    args << "time_base=1/25:pixel_aspect=1/1";

    std::cerr << args.str() << "\n";

    const auto ret{avfilter_graph_create_filter(&m_bufferSrc_ctx,bufferSrc,"in",
                                                  args.str().c_str(), nullptr,m_avFilterGraph)};
    if (ret < 0){
        throw std::runtime_error("Fail to create filter bufferSrc: " + AVHelper::av_get_err(ret) + "\n");
    }
}

void AVFilter_Demo::init_out_filter() noexcept(false)
{
    auto bufferSink{avfilter_get_by_name("buffersink")};

    if (!bufferSink){
        throw std::runtime_error("buffersink not found\n");
    }

    auto ret{avfilter_graph_create_filter(&m_Buffer_Sink_ctx, bufferSink, "out", nullptr,
                                                nullptr, m_avFilterGraph)};

    if (ret < 0){
        throw std::runtime_error("Fail to create filter sink filter: " + AVHelper::av_get_err(ret) + "\n");
    }

    constexpr AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };

    ret = av_opt_set_int_list(m_Buffer_Sink_ctx->priv,"pix_fmts",pix_fmts,pix_fmts[1],0);

    if (ret < 0){
        throw std::runtime_error(AVHelper::av_get_err(ret));
    }
}

void AVFilter_Demo::init_split_filter() noexcept(false)
{
    auto splitFilter {avfilter_get_by_name("split")};
    if (!splitFilter){
        throw std::runtime_error("split not failed\n");
    }

    const auto ret{avfilter_graph_create_filter(&m_splitFilter_ctx, splitFilter, "split", "outputs=2",
                                                nullptr, m_avFilterGraph)};

    if (ret < 0){
        throw std::runtime_error("Fail to create crop filter: " + AVHelper::av_get_err(ret) + "\n");
    }
}

void AVFilter_Demo::init_crop_filter() noexcept(false)
{
    auto cropFilter {avfilter_get_by_name("crop")};
    if (!cropFilter){
        throw std::runtime_error("crop not found\n");
    }

    std::stringstream args("out_w=iw:out_h=ih/2:x=0:y=0");

    const auto ret{avfilter_graph_create_filter(&m_cropFilter_ctx, cropFilter, "crop",
                                                args.str().c_str(), nullptr, m_avFilterGraph)};

    if (ret < 0){
        throw std::runtime_error("Fail to create crop filter: " + AVHelper::av_get_err(ret) + "\n");
    }
}

void AVFilter_Demo::init_vflip_filter() noexcept(false)
{
    auto vflipFilter {avfilter_get_by_name("vflip")};
    if (!vflipFilter){
        throw std::runtime_error("vflip not found\n");
    }

    const auto ret{avfilter_graph_create_filter(&m_vflipFilter_ctx, vflipFilter,
                                                "vflip", nullptr, nullptr, m_avFilterGraph)};
    if (ret < 0){
        throw std::runtime_error("Fail to create vflip filter: " + AVHelper::av_get_err(ret) + "\n");
    }
}

void AVFilter_Demo::init_overlay_filter( ) noexcept(false)
{
    auto overlayFilter {avfilter_get_by_name("overlay")};
    if (!overlayFilter){
        throw std::runtime_error("overlay not found\n");
    }

    const auto ret {avfilter_graph_create_filter(&m_overlayFilter_ctx, overlayFilter, "overlay",
                                                 "y=0:H/2", nullptr, m_avFilterGraph)};

    if (ret < 0){
        throw std::runtime_error("Fail to create overlay filter: " + AVHelper::av_get_err(ret) + "\n");
    }
}

void AVFilter_Demo::init_io_frame() noexcept(false)
{
    try {
        m_in_frame = new_ShareAVFrame();
    } catch (const std::exception &e) {
        throw std::runtime_error("new m_in_frame failed: " + std::string(e.what()) + "\n");
    }

    try {
        m_out_frame = new_ShareAVFrame();
    } catch (const std::exception &e) {
        throw std::runtime_error("new m_out_frame failed: " + std::string(e.what()) + "\n");
    }

    m_in_frame->m_frame->width = width;
    m_in_frame->m_frame->height = height;
    m_in_frame->m_frame->format = AV_PIX_FMT_YUV420P;

    const auto ret {av_image_fill_arrays(m_in_frame->m_frame->data,m_in_frame->m_frame->linesize,m_read_buffer,YUV_FMT,
                                         m_in_frame->m_frame->width,m_in_frame->m_frame->height,1)};
    if (ret < 0){
        throw std::runtime_error("alloc frame buffer failed: " + AVHelper::av_get_err(ret) + "\n");
    }
}

void AVFilter_Demo::link() noexcept(false)
{
    // src filter to split filter
    auto ret {avfilter_link(m_bufferSrc_ctx, 0, m_splitFilter_ctx, 0)};
    if (ret < 0) {
        throw std::runtime_error("Fail to link src filter and split filter: " + AVHelper::av_get_err(ret) + "\n");
    }

    // split filter's first pad to overlay filter's main pad
    ret = avfilter_link(m_splitFilter_ctx, 0, m_overlayFilter_ctx, 0);
    if (ret < 0){
        throw std::runtime_error("Fail to link split filter and overlay filter main pad: " + AVHelper::av_get_err(ret) + "\n");
    }

    // split filter's second pad to crop filter
    ret = avfilter_link(m_splitFilter_ctx, 1, m_cropFilter_ctx, 0);
    if (ret < 0){
        throw std::runtime_error("Fail to link split filter's second pad and crop filter: " + AVHelper::av_get_err(ret) + "\n");
    }

    // crop filter to vflip filter
    ret = avfilter_link(m_cropFilter_ctx, 0, m_vflipFilter_ctx, 0);
    if (ret < 0) {
        throw std::runtime_error("Fail to link crop filter and vflip filter: " + AVHelper::av_get_err(ret) + "\n");
    }

    // vflip filter to overlay filter's second pad
    ret = avfilter_link(m_vflipFilter_ctx, 0, m_overlayFilter_ctx, 1);
    if (ret < 0) {
        throw std::runtime_error("Fail to link vflip filter and overlay filter's second pad: " + AVHelper::av_get_err(ret) + "\n");
    }

    // overlay filter to sink filter
    ret = avfilter_link(m_overlayFilter_ctx, 0, m_Buffer_Sink_ctx, 0);
    if (ret < 0) {
        throw std::runtime_error("Fail to link overlay filter and sink filter: " + AVHelper::av_get_err(ret) + "\n");
    }
}

void AVFilter_Demo::check_filter_graph() noexcept(false)
{
    const auto ret{avfilter_graph_config(m_avFilterGraph, nullptr)};
    if (ret < 0){
        throw std::runtime_error("Fail in filter graph: " + AVHelper::av_get_err(ret) + "\n");
    }
}

AVFilter_Demo_sp_type AVFilter_Demo::create(const std::string &in,const std::string &out) noexcept(false)
{
    AVFilter_Demo_sp_type obj;
    try {
        obj = std::move(AVFilter_Demo_sp_type(new AVFilter_Demo(in,out)));
    } catch (const std::bad_alloc &e) {
        throw std::runtime_error("new AVFilter_Demo failed: " + std::string (e.what()) + "\n");
    }

    try {
        obj->Construct();
        return obj;
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw std::runtime_error("AVFilter_Demo Construct failed: " +
                                    std::string (e.what()) + "\n");
    }
}

void AVFilter_Demo::DeConstruct() noexcept(false)
{
    m_mem_pool.deallocate(m_read_buffer,m_read_size);
    m_in_yuv_file.close();
    m_out_yuv_file.close();
    avfilter_graph_free(&m_avFilterGraph);
}

AVFilter_Demo::~AVFilter_Demo()
{
    DeConstruct();
}

void AVFilter_Demo::exec() noexcept(false)
{
    for (;;) {

        m_in_yuv_file.read(reinterpret_cast<char *>(m_read_buffer),static_cast<int64_t>(m_read_size));

        if (m_in_yuv_file.eof() || m_in_yuv_file.gcount() < m_read_size) {
            std::cerr << "m_in_yuv_file read finish\n";
            break;
        }

        auto ret {av_buffersrc_add_frame(m_bufferSrc_ctx,m_in_frame->m_frame)};

        if (ret < 0) {
            throw std::runtime_error("Error while add frame: " + AVHelper::av_get_err(ret) + "\n");
        }

        ret = av_buffersink_get_frame(m_Buffer_Sink_ctx, m_out_frame->m_frame);
        if (ret < 0){
            throw std::runtime_error("av_buffersink_get_frame failed: " + AVHelper::av_get_err(ret) + "\n");
        }

        if (YUV_FMT == m_out_frame->m_frame->format){
            for (int i {}; i < m_out_frame->m_frame->height; ++i) {
                m_out_yuv_file.write(reinterpret_cast<const char *>(m_out_frame->m_frame->data[0] + m_out_frame->m_frame->linesize[0] * i),
                                     m_out_frame->m_frame->linesize[0]);
            }

            for (int i {}; i < m_out_frame->m_frame->height / 2; ++i) {
                m_out_yuv_file.write(reinterpret_cast<const char*>(m_out_frame->m_frame->data[1] + m_out_frame->m_frame->linesize[1] * i),
                                     m_out_frame->m_frame->linesize[1]);
            }

            for (int i {}; i < m_out_frame->m_frame->height / 2; ++i) {
                m_out_yuv_file.write(reinterpret_cast<const char*>(m_out_frame->m_frame->data[2] + m_out_frame->m_frame->linesize[2] * i),
                                     m_out_frame->m_frame->linesize[2]);
            }
        }

        if (!(++frame_count % 25)){
            std::cerr << "Process " << frame_count << " frame!\n";
        }
        av_frame_unref(m_out_frame->m_frame);
    }
}

AVFilter_Demo_sp_type new_AVFilter_Demo(const std::string &in,const std::string &out) noexcept(false)
{
    return AVFilter_Demo::create(in,out);
}
