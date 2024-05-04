//
// Created by Administrator on 2024/5/4.
//

#ifndef INC_17_AUDIO_MIX_AUDIO_MIX_HPP
#define INC_17_AUDIO_MIX_AUDIO_MIX_HPP

extern "C"{
#include <libavfilter/avfilter.h>
};

#include <memory>
#include <map>
#include <fstream>
#include <atomic>
#include "AudioInfo.hpp"

class Audio_Mix final {

    explicit Audio_Mix(const std::string & ,
                       const std::string & ,
                       const std::string & );
    void Construct() noexcept(false);
    void DeConstruct() noexcept(true);

    void init_audio_mix_filter(const std::string &) noexcept(false);
    void init_audio_format_filter() noexcept(false);
    void init_audio_sink_buffer_filter() noexcept(false);
    void init_audio_input_info() noexcept(false);
    void link_filter() noexcept(false);
    void config_FilterGraph() noexcept(false);

public:
    using Audio_Mix_sp_type = std::shared_ptr<Audio_Mix>;
    Audio_Mix(const Audio_Mix&) = delete;
    Audio_Mix& operator=(const Audio_Mix&) = delete;
    static Audio_Mix_sp_type create(const std::string & ,
                                    const std::string & ,
                                    const std::string & ) noexcept(false);
    ~Audio_Mix();
    void init(const std::string & = "longest") noexcept(false);
    void push_in_audio_info(const int &,AudioInfo &&) noexcept(false);
    void push_out_audio_info(AudioInfo &&) noexcept(false);
//    void add_frame() noexcept(false);
//    void add_output() noexcept(false);
//    //void get_frame() noexcept(false);

private:
    std::ifstream m_input_file_1,m_input_file_2;
    std::ofstream m_output_file;
    AVFilterGraph *m_FilterGraph{};

    AVFilterContext *m_mix_filter_ctx{},
    *m_sink_buffer_filter_ctx{};

    AudioInfo_sp_type m_output_filter;

    std::map<int,AudioInfo> m_Audio_infos;
    std::atomic_bool m_initialized{};
};

using Audio_Mix_sp_type = Audio_Mix::Audio_Mix_sp_type;

Audio_Mix_sp_type new_Audio_Mix(const std::string & ,
                                const std::string & ,
                                const std::string & ) noexcept(false);

#endif //INC_17_AUDIO_MIX_AUDIO_MIX_HPP
