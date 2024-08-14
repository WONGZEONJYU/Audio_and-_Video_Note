//
// Created by wong on 2024/7/27.
//

#ifndef XPLAY2_XDEMUX_HPP
#define XPLAY2_XDEMUX_HPP

#include <mutex>
#include <atomic>
#include <memory>
#include <vector>
#include "XHelper.hpp"

struct AVFormatContext;
struct AVStream;
class XAVPacket;
class XAVCodecParameters;

using XAVCodecParameters_sptr = typename std::shared_ptr<XAVCodecParameters>;

class XDemux {

    void show_audio_info() const noexcept(true);
    void show_video_info()  noexcept(true);
    void DeConstruct() noexcept(true);
    static int io_callback(void *);
public:
    explicit XDemux();
    /**
     * 打开文件,打开失败抛出异常,分配失败为异常
     */
    virtual void Open(const std::string &) noexcept(false);

    /**
     *  读取packed,std::shared_ptr<XAVPacket>可能出现空,有分配异常
     * @return
     */
    virtual std::shared_ptr<XAVPacket> Read() noexcept(false);

    /**
     * 拷贝视频解码参数集,无需手动释放,有分配异常,没有则返回空
     * @return Video_CodecParameters
     */
    virtual XAVCodecParameters_sptr Copy_Present_VideoCodecParam() noexcept(false);

    /**
     * 拷贝音频解码参数集,无需手动释放,有分配异常,没有则返回空
     * @return Audio_CodecParameters
     */
    virtual XAVCodecParameters_sptr Copy_Present_AudioCodecParam() noexcept(false);

    /**
     * 判断是否为音频
     * @return
     */
    virtual bool is_Audio(const std::shared_ptr<XAVPacket> &) noexcept(true);

    /**
     * Seek位置,按百分比
     * @return
     */
    virtual bool Seek(const double &) noexcept(true);

    /**
     * 刷新m_av_fmt_ctx
     */
    virtual void Clear() noexcept(true);
    /**
     * 关闭m_av_fmt_ctx,重新打开文件之前,需手动关闭
     */
    virtual void Close() noexcept(true);

    [[nodiscard]] virtual int Present_Video_Index() const {
        return m_Present_Video_index;
    }

    [[nodiscard]] virtual int Present_Audio_Index() const {
        return m_Present_Audio_index;
    }

    [[nodiscard]] virtual int64_t totalMS() const noexcept(true){
        return m_totalMS;
    }

protected:
    std::mutex m_mux;
    std::vector<int> m_stream_indices;
    AVFormatContext *m_av_fmt_ctx{};
    AVStream **m_streams{},
            *m_Present_Video_st{},
            *m_Present_Audio_st{};
    int64_t m_totalMS{};
    uint32_t m_nb_streams{};
    std::atomic_int m_Present_Video_index{-1},
        m_Present_Audio_index{-1};
    std::atomic_int m_is_exit{};
private:
    static std::atomic_uint64_t sm_init_times;
    static std::mutex sm_mux;

public:
    virtual ~XDemux();
    X_DISABLE_COPY(XDemux)
};

#endif
