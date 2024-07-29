//
// Created by wong on 2024/7/27.
//

#ifndef XPLAY2_XDEMUX_HPP
#define XPLAY2_XDEMUX_HPP

#include <string>
#include <mutex>
#include <atomic>
#include <memory>

struct AVFormatContext;
struct AVStream;
struct XAVPacket;

class XDemux {

    void show_audio_info() const noexcept(true);
    void show_video_info() const noexcept(true);
    void Deconstruct() noexcept(true);

public:
    explicit XDemux();
    virtual void Open(const std::string &) noexcept(false);
    virtual std::shared_ptr<XAVPacket> Read() noexcept(false);
    virtual ~XDemux();

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
    uint32_t *m_stream_indices{};

private:
    static std::atomic_uint64_t sm_init_times;
    static std::mutex sm_mux;
    int64_t m_totalMS{};
    std::atomic_bool is_init{};
    uint32_t m_nb_streams{};

public:
    XDemux(const XDemux &) = delete;
    XDemux& operator=(const XDemux&) = delete;
};

#endif
