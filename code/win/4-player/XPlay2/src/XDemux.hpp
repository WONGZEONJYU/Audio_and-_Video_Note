//
// Created by wong on 2024/7/27.
//

#ifndef XPLAY2_XDEMUX_HPP
#define XPLAY2_XDEMUX_HPP

#include <string>
#include <mutex>
#include <atomic>
#include <memory>
#include <vector>
#include "XHelper.h"

struct AVFormatContext;
struct AVStream;
struct XAVPacket;
class XAVCodecParameters;

using XAVCodecParameters_sptr_container = std::vector<std::shared_ptr<XAVCodecParameters>>;
using XAVCodecParameters_container_sprt = std::shared_ptr<XAVCodecParameters_sptr_container>;

class XDemux {

    void show_audio_info() const noexcept(true);
    void show_video_info() const noexcept(true);
    void Deconstruct() noexcept(true);

public:
    explicit XDemux();
    virtual void Open(const std::string &) noexcept(false);
    virtual std::shared_ptr<XAVPacket> Read() noexcept(false);
    virtual XAVCodecParameters_container_sprt copy_ALLCodec_Parameters() noexcept(false);
    virtual bool Seek(const double &) noexcept(true);
    virtual void Clear() noexcept(true);
    virtual void Close() noexcept(true);
    [[nodiscard]] auto totalMS() const noexcept(true){
        return m_totalMS;
    }

protected:
    std::mutex m_mux;
    AVFormatContext *m_av_fmt_ctx{};
//    AVStream *m_audio_stream{},
//            *m_video_stream{};
//    int m_audio_stream_index{},
//        m_video_stream_index{};
    AVStream **m_streams{};
    int *m_stream_indices{};

private:
    static std::atomic_uint64_t sm_init_times;
    static std::mutex sm_mux;
    int64_t m_totalMS{};
    std::atomic_bool is_init{};
    uint32_t m_nb_streams{};

public:
    virtual ~XDemux();
    X_DISABLE_COPY(XDemux)
};

#endif