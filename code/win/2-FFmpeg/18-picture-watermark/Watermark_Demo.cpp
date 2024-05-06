//
// Created by Administrator on 2024/5/6.
//

#include "Watermark_Demo.hpp"
#include <sstream>
#include <fstream>
#include "AVHelper.h"

Watermark_Demo::Watermark_Demo(const char **argv) noexcept(true):
        m_argv(argv),
        m_avFilterGraph(avfilter_graph_alloc())
{
}

void Watermark_Demo::Construct() noexcept(false)
{
    if (!m_avFilterGraph){
        throw std::runtime_error("alloc AVFilterGraph error\n");
    }
    get_picture();
    init_filters();
}

ShareAVFrame_sp_type Watermark_Demo::get_frame_from_jpeg_file(const std::string &file) noexcept(false)
{
    AVFormatContext *format_ctx{};
    AVCodecContext *codecContext{};

    const Destroyer d([&]{
        avformat_close_input(&format_ctx);
        avcodec_free_context(&codecContext);
    });

    auto ret {avformat_open_input(&format_ctx,file.c_str(), nullptr, nullptr)};
    if (ret < 0){
        d.~Destroyer();
        throw std::runtime_error("Error: avformat_open_input failed: " + file + "\n");
    }

    ret = avformat_find_stream_info(format_ctx, nullptr);
    if (ret < 0){
        d.~Destroyer();
        throw std::runtime_error(file + " avformat_find_stream_info error: " + AVHelper::av_get_err(ret) + "\n");
    }

    const AVCodec *codec{};
    const auto video_index {av_find_best_stream(format_ctx,
                                                AVMEDIA_TYPE_VIDEO,
                                                -1,
                                                -1,
                                                &codec,0)};
    if (video_index){
        d.~Destroyer();
        throw std::runtime_error(file + " video_index not found: " + AVHelper::av_get_err(video_index) + "\n");
    }

    codecContext = avcodec_alloc_context3(codec);

    if (!codecContext){
        d.~Destroyer();
        throw std::runtime_error(file + " avcodec_alloc_context3 failed\n");
    }

    ret = avcodec_open2(codecContext, codec, nullptr);
    if (ret < 0){
        throw std::runtime_error(file +  "avcodec_open2 error: " + AVHelper::av_get_err(ret) + "\n");
    }

    AVPacket pkt{};
    ret = av_read_frame(format_ctx,&pkt);
    if (ret < 0){
        throw std::runtime_error(file + "av_read_frame error: " + AVHelper::av_get_err(ret) + "\n");
    }

    ShareAVFrame_sp_type frame;

    try {
        AVHelper::decode("jpeg",codecContext,&pkt,[&frame](const auto &avFrame){
            frame = avFrame;
        });

    } catch (const std::system_error &e) {
        ret = e.code().value();
        frame.reset();
        d.~Destroyer();
        throw std::runtime_error(file + "decode error: " + AVHelper::av_get_err(ret));
    } catch (const std::runtime_error &e) {
        throw std::runtime_error(e.what());
    }

    return frame;
}

void Watermark_Demo::get_picture() noexcept(false)
{
    m_main_frame = get_frame_from_jpeg_file(m_argv[1]);
    m_logo_frame = get_frame_from_jpeg_file(m_argv[2]);
    m_out_frame = new_ShareAVFrame();
}

void Watermark_Demo::init_filters() noexcept(false)
{
    std::stringstream args;

    args << "buffer=video_size=" << m_main_frame->m_frame->width << "x" << m_main_frame->m_frame->height <<
            ":pix_fmt=" << m_main_frame->m_frame->format <<
            ":time_base=1/25:pixel_aspect=" <<
            m_main_frame->m_frame->sample_aspect_ratio.num << "/" << m_main_frame->m_frame->sample_aspect_ratio.den <<
            "[main];";

    args << "buffer=video_size=" << m_logo_frame->m_frame->width << "x" << m_logo_frame->m_frame->height <<
            ":pix_fmt=" << m_logo_frame->m_frame->format <<
            ":time_base=1/25:pixel_aspect=" <<
            m_logo_frame->m_frame->sample_aspect_ratio.num << "/" << m_logo_frame->m_frame->sample_aspect_ratio.den <<
            "[logo];";

    args << "[main][logo]overlay=0:200[result];";
    args << "[result]buffersink";

    std::cerr << args.str() << "\n";
    AVFilterInOut *inputs {},*outputs {};

    auto ret {avfilter_graph_parse2(m_avFilterGraph, args.str().c_str(), &inputs, &outputs)};
    if (ret < 0) {
        throw std::runtime_error("Cannot parse graph\n");
    }

    ret = avfilter_graph_config(m_avFilterGraph, nullptr);
    if (ret < 0){
        throw std::runtime_error("Cannot configure graph: " + AVHelper::av_get_err(ret) + "\n");
    }

    m_main_src_buffer_filter_ctx = avfilter_graph_get_filter(m_avFilterGraph,m_avFilterGraph->filters[0]->name);
    m_logo_src_buffer_filter_ctx = avfilter_graph_get_filter(m_avFilterGraph,m_avFilterGraph->filters[1]->name);
    m_sink_buffer_filter_ctx = avfilter_graph_get_filter(m_avFilterGraph,m_avFilterGraph->filters[3]->name);

    if (!m_main_src_buffer_filter_ctx){
        throw std::runtime_error("buffer_filter_ctx Failed to obtain\n");
    }

    if (!m_logo_src_buffer_filter_ctx){
        throw std::runtime_error("logo_src_buffer_filter_ctx Failed to obtain\n");
    }

    if (!m_sink_buffer_filter_ctx){
        throw std::runtime_error("sink_buffer_filter_ctx Failed to obtain\n");
    }
}

void Watermark_Demo::main_picture_mix_logo() noexcept(false)
{
    auto ret {av_buffersrc_add_frame(m_main_src_buffer_filter_ctx,m_main_frame->m_frame)};
    if (ret < 0){
        throw std::runtime_error("add main frame error: " + AVHelper::av_get_err(ret) + "\n");
    }

    ret = av_buffersrc_add_frame(m_logo_src_buffer_filter_ctx,m_logo_frame->m_frame);
    if (ret < 0){
        throw std::runtime_error("add logo frame error: " + AVHelper::av_get_err(ret) + "\n");
    }

    ret = av_buffersink_get_frame(m_sink_buffer_filter_ctx,m_out_frame->m_frame);
    if (ret < 0){
        throw std::runtime_error("get out frame error: " + AVHelper::av_get_err(ret) + "\n");
    }
}

void Watermark_Demo::save_picture() noexcept(false)
{
    std::cerr << m_argv[3] << "\n";
    std::ofstream out_file(m_argv[3],std::ios::binary);

    AVCodecContext *avCodecContext{};

    Destroyer d([&]{
        out_file.close();
        avcodec_free_context(&avCodecContext);
    });

    if (!out_file){
        d.~Destroyer();
        throw std::runtime_error("open out file error\n");
    }

    auto codec {avcodec_find_encoder(AV_CODEC_ID_MJPEG)};
    if (!codec){
        d.~Destroyer();
        throw std::runtime_error("AV_CODEC_ID_MJPEG not found\n");
    }

    avCodecContext = avcodec_alloc_context3(codec);

    if (!avCodecContext){
        d.~Destroyer();
        throw std::runtime_error(std::string(__FUNCTION__) + " avcodec_alloc_context3 error\n" );
    }

    avCodecContext->pix_fmt = AV_PIX_FMT_YUVJ420P;
    avCodecContext->width = m_out_frame->m_frame->width;
    avCodecContext->height = m_out_frame->m_frame->height;
    avCodecContext->time_base = {1,25};
    avCodecContext->framerate = {25,1};
    AVDictionary *encoder_opts {};
    //    av_dict_set(&encoder_opts, "qscale:v", "2", 0);
    av_dict_set(&encoder_opts, "flags", "+qscale", 0);
    av_dict_set(&encoder_opts, "qmax", "2", 0);
    av_dict_set(&encoder_opts, "qmin", "2", 0);

    auto ret {avcodec_open2(avCodecContext,codec,&encoder_opts)};
    if (ret < 0){
        d.~Destroyer();
        av_dict_free(&encoder_opts);
        throw std::runtime_error(std::string(__FUNCTION__ ) + " avcodec_open2 error: " + AVHelper::av_get_err(ret) + "\n");
    }
    av_dict_free(&encoder_opts);

    try {
        AVHelper::encode("jpeg",avCodecContext,m_out_frame->m_frame,
                         [&out_file](const auto &pkt){
                out_file.write(reinterpret_cast<const char *>(pkt->m_packet->data),pkt->m_packet->size);
        });

    } catch (const std::system_error &e) {
        ret = e.code().value();
        throw std::runtime_error("encode error: " + AVHelper::av_get_err(ret) + "\n");
    } catch (const std::runtime_error &e) {
        throw std::runtime_error(e.what());
    }
}

void Watermark_Demo::exec() noexcept(false)
{
    main_picture_mix_logo();
    save_picture();
}

void Watermark_Demo::DeConstruct() noexcept(true)
{
    avfilter_graph_free(&m_avFilterGraph);
}

Watermark_Demo::~Watermark_Demo()
{
    DeConstruct();
}

Watermark_Demo_sp_type Watermark_Demo::create(const char **argv)
{
    Watermark_Demo_sp_type obj;

    try {
        obj.reset(new Watermark_Demo(argv));
        obj->Construct();
        return obj;
    } catch (const std::bad_alloc &e) {
        throw std::runtime_error("new Watermark_Demo error: " + std::string(e.what()) + "\n");
    }catch (const std::runtime_error &e) {
        obj.reset();
        throw std::runtime_error("Watermark_Demo Construct error: " + std::string(e.what()) + "\n");
    }
}

Watermark_Demo_sp_type new_Watermark_Demo(const char **argv) noexcept(false)
{
    return Watermark_Demo::create(argv);
}
