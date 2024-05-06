//
// Created by Administrator on 2024/5/4.
//

extern "C"{
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/samplefmt.h>
}

#include <sstream>
#include <algorithm>
#include "Audio_Mix.hpp"
#include "AVHelper.h"

Audio_Mix::Audio_Mix() noexcept(true):
        m_FilterGraph(avfilter_graph_alloc())
{
}

void Audio_Mix::Construct() noexcept(false)
{
    if (!m_FilterGraph){
        throw std::runtime_error("alloc FilterGraph error\n");
    }
}

void Audio_Mix::init_audio_mix_filter(const std::string &duration) noexcept(false)
{
    const auto amix_filter {avfilter_get_by_name("amix")};
    if (!amix_filter){
        throw std::runtime_error("amix_filter not found\n");
    }

    std::stringstream args;
    args << "inputs=" << m_Audio_infos.size() << ":";
    args << "duration=" << duration << ":";
    args << "dropout_transition=0";

    std::cerr << args.str() << "\n";

    auto ret {avfilter_graph_create_filter(&m_mix_filter_ctx,amix_filter,
                                           "amix",
                                           args.str().c_str(),
                                           nullptr,
                                           m_FilterGraph)};
    if (ret < 0){
        throw std::runtime_error("avfilter_graph_create_filter(amix) error :" + AVHelper::av_get_err(ret) + "\n");
    }
}

void Audio_Mix::init_audio_format_filter() noexcept(false)
{
    if (!m_output_filter){
        throw std::runtime_error("please add output audio info\n");
    }

    const auto aformat_filter{avfilter_get_by_name("aformat")};
    if (!aformat_filter){
        throw std::runtime_error("aformat_filter not found\n");
    }

    std::stringstream args;
    args << "sample_rates=" << m_output_filter->m_sample_rate << ":";
    args << "sample_fmts=" << av_get_sample_fmt_name(m_output_filter->m_sample_fmt) << ":";
    args << "channel_layouts=" << m_output_filter->m_ch_layout.u.mask;

    std::cerr << args.str() << "\n";

    const auto ret {avfilter_graph_create_filter(&m_output_filter->m_filter_ctx,aformat_filter,
                                                 "aformat",
                                                 args.str().c_str(),
                                                 nullptr,
                                                 m_FilterGraph)};
    if (ret < 0){
        throw std::runtime_error("avfilter_graph_create_filter (aformat) error: " + AVHelper::av_get_err(ret) + "\n");
    }
}

void Audio_Mix::init_audio_sink_buffer_filter() noexcept(false)
{
    const auto sink_filter{avfilter_get_by_name("abuffersink")};

    if (!sink_filter){
        throw std::runtime_error("abuffersink_filter not found\n");
    }

    const auto ret {avfilter_graph_create_filter(&m_sink_buffer_filter_ctx,sink_filter,
                                       "abuffersink",
                                       nullptr,
                                       nullptr,
                                       m_FilterGraph)};
    if (ret < 0){
        throw std::runtime_error("avfilter_graph_create_filter (abuffersink) error: " + AVHelper::av_get_err(ret) + "\n");
    }
}

void Audio_Mix::init_audio_input_info() noexcept(false)
{
    const auto abuffer_filter{avfilter_get_by_name("abuffer")};

    if (!abuffer_filter){
        throw std::runtime_error("abuffer_filter not found\n");
    }

    for (auto & item : m_Audio_infos) {

        std::stringstream args;
        args << "sample_rate=" << item.second.m_sample_rate << ":";
        args << "sample_fmt=" << av_get_sample_fmt_name(item.second.m_sample_fmt) << ":";
        args << "channel_layout=" << item.second.m_ch_layout.u.mask;

        std::cerr << args.str() << "\n";

        auto ret {avfilter_graph_create_filter(&item.second.m_filter_ctx,abuffer_filter,
                                                     item.second.m_name.c_str(),
                                                     args.str().c_str(),
                                                     nullptr,
                                                     m_FilterGraph)};

        if (ret < 0){
            throw std::runtime_error("avfilter_graph_create_filter (" + item.second.m_name + ")" + "\n");
        }

        ret = avfilter_link(item.second.m_filter_ctx,0,m_mix_filter_ctx,item.first);
        if (ret < 0){
            throw std::runtime_error("[AudioMixer] avfilter_link(abuffer(" + std::to_string(item.first) + "), amix) failed.\n");
        }
    }
}

void Audio_Mix::link_filter() noexcept(false)
{
    auto ret {avfilter_link(m_mix_filter_ctx,0,m_output_filter->m_filter_ctx,0)};
    if (ret < 0){
        throw std::runtime_error("mix_filter_ctx link output_filter_ctx error: " + AVHelper::av_get_err(ret) + "\n");
    }

    ret = avfilter_link(m_output_filter->m_filter_ctx,0,m_sink_buffer_filter_ctx,0);
    if (ret < 0){
        throw std::runtime_error("output_filter_ctx link sink_buffer_filter_ctx error: " + AVHelper::av_get_err(ret) + "\n");
    }
}

void Audio_Mix::config_FilterGraph() noexcept(false)
{
    const auto ret {avfilter_graph_config(m_FilterGraph, nullptr)};
    if (ret < 0){
        throw std::runtime_error("avfilter_graph_config failed: " + AVHelper::av_get_err(ret) + "\n");
    }
}

void Audio_Mix::init(const std::string &duration) noexcept(false)
{
    if (m_initialized){
        throw std::runtime_error("initialized\n");
    }

    if (m_Audio_infos.empty()){
        throw std::runtime_error("please add audio info\n");
    }

    init_audio_mix_filter(duration);
    init_audio_format_filter();
    init_audio_sink_buffer_filter();
    init_audio_input_info();
    link_filter();
    config_FilterGraph();

    AVHelper::av_filter_graph_dump(m_FilterGraph,"m_FilterGraph.txt");
    std::cerr << "init finish\n";
    m_initialized = true;
}

void Audio_Mix::push_in_audio_info(const int &index,AudioInfo &&info) noexcept(false)
{
    if (m_Audio_infos.find(index) != m_Audio_infos.end()){
        throw std::runtime_error("index:\t" + std::to_string(index) + "\texisted");
    }
    m_Audio_infos.insert({index,std::move(info)});
}

void Audio_Mix::push_out_audio_info(AudioInfo &&info) noexcept(false)
{
    if (m_output_filter){
        throw std::runtime_error("output_filter existed\n");
    } else{
        try {
            m_output_filter = std::make_shared<AudioInfo>(std::move(info.m_name),info.m_sample_rate,
                                                          info.m_sample_fmt,info.m_ch_layout);
        } catch (const std::bad_alloc &e) {
            throw std::runtime_error("push_out_audio_info error： " + std::string (e.what()) + "\n");
        }
    }
}

void Audio_Mix::add_frame(const int &index,const uint8_t *src,const size_t& pcm_size) noexcept(false)
{
    if (!m_initialized){
        throw std::runtime_error("Uninitialized\n");
    }

    const auto &item {m_Audio_infos.at(index)};

    AVFrame *frame{};
    ShareAVFrame_sp_type shareAvFrame;

    if (src && pcm_size > 0) {
        shareAvFrame = new_ShareAVFrame();
        shareAvFrame->m_frame->sample_rate = item.m_sample_rate;
        shareAvFrame->m_frame->ch_layout = item.m_ch_layout;
        shareAvFrame->m_frame->format = item.m_sample_fmt;
        const auto nb_samples{pcm_size /
                                                av_get_bytes_per_sample(item.m_sample_fmt) /
                                                item.m_ch_layout.nb_channels};

        shareAvFrame->m_frame->nb_samples = static_cast<int>(nb_samples);

        const auto ret {av_samples_fill_arrays(shareAvFrame->m_frame->data,
                                               shareAvFrame->m_frame->linesize,
                                               src,
                                               shareAvFrame->m_frame->ch_layout.nb_channels,
                                               shareAvFrame->m_frame->nb_samples,
                                               static_cast<AVSampleFormat>(shareAvFrame->m_frame->format),
                                               0)};
        if (ret < 0){
            throw std::runtime_error("av_samples_fill_arrays failed: " + AVHelper::av_get_err(ret) + "\t" +
                                             item.m_name + "\n");
        }

        frame = shareAvFrame->m_frame;
    }

    const auto ret {av_buffersrc_add_frame(item.m_filter_ctx,frame)};
    if (ret < 0){
        throw std::runtime_error("av_buffersrc_add_frame(" + std::to_string(index) + ")error: " +
                                AVHelper::av_get_err(ret) + "\n");
    }
}

int Audio_Mix::get_frame(uint8_t *dst) noexcept(false)
{
    if (!m_initialized){
        throw std::runtime_error("Uninitialized\n");
    }

    if (!dst){
        throw std::runtime_error("dst is empty\n");
    }

    auto frame{new_ShareAVFrame()};

    const auto ret {av_buffersink_get_frame(m_sink_buffer_filter_ctx, frame->m_frame)};

    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
        return ret;
    }else if (ret < 0){
        throw std::runtime_error("av_buffersink_get_frame error: " + AVHelper::av_get_err(ret) + "\n");
    }

    const auto size {av_samples_get_buffer_size(nullptr,
                                               frame->m_frame->ch_layout.nb_channels,
                                               frame->m_frame->nb_samples,
                                               static_cast<AVSampleFormat>(frame->m_frame->format),
                                               0)};
    if (size < 0){
        throw std::runtime_error("av_samples_get_buffer_size error: " + AVHelper::av_get_err(size) + "\n");
    }

    std::copy_n(frame->m_frame->extended_data[0],size,dst);

    return size;
}

void Audio_Mix::DeConstruct() noexcept(true)
{
    avfilter_graph_free(&m_FilterGraph);
}

Audio_Mix::~Audio_Mix()
{
    DeConstruct();
}

Audio_Mix_sp_type Audio_Mix::create() noexcept(false)
{
    Audio_Mix_sp_type obj;
    try {
        obj = std::move(Audio_Mix_sp_type(new Audio_Mix()));
    } catch (const std::bad_alloc &e) {
        throw std::runtime_error("new Audio_Mix error: " + std::string(e.what()) + "\n");
    }

    try {
        obj->Construct();
        return obj;
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw std::runtime_error("Audio_Mix construct error: " + std::string (e.what()) + "\n");
    }
}

Audio_Mix_sp_type new_Audio_Mix() noexcept(false)
{
    return Audio_Mix::create();
}
