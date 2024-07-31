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
#include <unordered_map>
#include "XHelper.h"

struct AVFormatContext;
struct AVStream;
struct XAVPacket;
class XAVCodecParameters;

using XAVCodecParameters_sptr = typename std::shared_ptr<XAVCodecParameters>;
using XAVCodecParameters_sptr_container = typename std::unordered_map<int,XAVCodecParameters_sptr>;
using XAVCodecParameters_sptr_container_sptr = typename std::shared_ptr<XAVCodecParameters_sptr_container>;

class XDemux {

    void show_audio_info() const noexcept(true);
    void show_video_info() const noexcept(true);
    void Deconstruct() noexcept(true);

public:
    explicit XDemux();
    //打开文件,打开失败抛出异常
    virtual void Open(const std::string &) noexcept(false);
    //读取packed,std::shared_ptr<XAVPacket>可能为空,有异常
    virtual std::shared_ptr<XAVPacket> Read() noexcept(false);
    //拷贝解码参数,无需手动释放,没打开文件则抛出异常
    virtual XAVCodecParameters_sptr_container_sptr copy_ALLCodec_Parameters() noexcept(false);

    virtual bool is_Audio(const std::shared_ptr<XAVPacket> &) noexcept(true);
    //Seek位置,按百分比
    virtual bool Seek(const double &) noexcept(true);
    //刷新m_av_fmt_ctx
    virtual void Clear() noexcept(true);
    //关闭m_av_fmt_ctx,重新打开文件之前,需手动关闭
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
    uint32_t m_nb_streams{};

public:
    virtual ~XDemux();
    X_DISABLE_COPY(XDemux)
};

#endif
