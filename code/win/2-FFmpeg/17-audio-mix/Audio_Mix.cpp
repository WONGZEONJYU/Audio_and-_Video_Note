//
// Created by Administrator on 2024/5/4.
//
#include <sstream>
#include "Audio_Mix.hpp"
#include "AVHelper.h"

Audio_Mix::Audio_Mix(const std::string & in_1,
                   const std::string & in_2,
                   const std::string & out):
        m_input_file_1(in_1,std::ios::binary),
        m_input_file_2(in_2,std::ios::binary),
        m_output_file(out,std::ios::binary),
        m_FilterGraph(avfilter_graph_alloc())
{
}

void Audio_Mix::Construct() noexcept(false)
{
    if (!m_input_file_1){
        throw std::runtime_error("open file_1 failed\n");
    }

    if (!m_input_file_2){
        throw std::runtime_error("open file_2 failed\n");
    }

    if (!m_output_file){
        throw std::runtime_error("open output failed\n");
    }

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

    AVHelper::avfilter_graph_dump(m_FilterGraph,"m_FilterGraph.txt");
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

void Audio_Mix::DeConstruct() noexcept(true)
{
    m_input_file_1.close();
    m_input_file_2.close();
    m_output_file.close();
    avfilter_graph_free(&m_FilterGraph);
}

Audio_Mix::~Audio_Mix()
{
    DeConstruct();
}

Audio_Mix_sp_type Audio_Mix::create(const std::string & in_1,
                                const std::string & in_2,
                                const std::string & out) noexcept(false)
{
    Audio_Mix_sp_type obj;
    try {
        obj = std::move(Audio_Mix_sp_type(new Audio_Mix(in_1,in_2,out)));
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

Audio_Mix_sp_type new_Audio_Mix(const std::string & in_1,
                                const std::string & in_2,
                                const std::string & out) noexcept(false)
{
    return Audio_Mix::create(in_1,in_2,out);
}
