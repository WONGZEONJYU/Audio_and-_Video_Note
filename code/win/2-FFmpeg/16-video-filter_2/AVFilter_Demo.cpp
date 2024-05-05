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
m_avFilterGraph(avfilter_graph_alloc())
{

}

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

    try {
        m_read_buffer = static_cast<decltype(m_read_buffer)>(m_mem_pool.allocate(m_read_size));
    } catch (const std::exception &e) {
        throw std::runtime_error("alloc read_buffer error: " + std::string(e.what()) + "\n");
    }

    init_filters();
    check_filter_graph();
    get_filters_ctx();
    init_io_frame();
    AVHelper::avfilter_graph_dump(m_avFilterGraph,"graphFile.txt");
    std::cerr << "init finish\n";
}

void AVFilter_Demo::init_filters() noexcept(false)
{
    AVFilterInOut *inputs {};
    AVFilterInOut *outputs {};

    std::stringstream args;
    args << "buffer=video_size=" << width << "x" << height <<
            ":pix_fmt=" << YUV_FMT <<
            ":time_base=" << time_base.num << "/" << time_base.den <<
            ":pixel_aspect=1/1[v0];"; // Parsed_buffer_0;
    args << "[v0]split[main][tmp];";        // Parsed_split_1
    args << "[tmp]crop=iw:ih/2:0:0,vflip[flip];";   // Parsed_crop_2 Parsed_vflip_3
    args << "[main][flip]overlay=0:H/2[result];"; // Parsed_overlay_4
    args << "[result]buffersink"; // Parsed_buffersink_5

    std::cerr << args.str() << "\n";

    const auto ret { avfilter_graph_parse2(m_avFilterGraph, args.str().c_str(), &inputs, &outputs) };
    if (ret < 0){
        std::cerr << "Cannot parse graph: " << AVHelper::av_get_err(ret) << "\n";
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

void AVFilter_Demo::check_filter_graph() noexcept(false)
{
    const auto ret{avfilter_graph_config(m_avFilterGraph, nullptr)};
    if (ret < 0){
        throw std::runtime_error("Fail in filter graph: " + AVHelper::av_get_err(ret) + "\n");
    }
}

void AVFilter_Demo::get_filters_ctx() noexcept(false)
{
    m_bufferSrc_ctx = avfilter_graph_get_filter(m_avFilterGraph, m_avFilterGraph->filters[0]->name);
    if (!m_bufferSrc_ctx){
        throw std::runtime_error("get bufferSrc_ctx error\n");
    }

    m_Buffer_Sink_ctx = avfilter_graph_get_filter(m_avFilterGraph, m_avFilterGraph->filters[5]->name);
    if (!m_Buffer_Sink_ctx){
        throw std::runtime_error("get Buffer_Sink_ctx error\n");
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
