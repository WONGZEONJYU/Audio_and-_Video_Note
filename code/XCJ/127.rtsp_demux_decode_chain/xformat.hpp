//
// Created by wong on 2024/9/21.
//

#ifndef INC_124_TEST_XFORMAT_XFORMAT_HPP
#define INC_124_TEST_XFORMAT_XFORMAT_HPP

#include "xhelper.hpp"
#include <mutex>
#include <atomic>
#include <memory>

struct AVFormatContext;
struct AVCodecParameters;
struct AVCodecContext;
class XAVPacket;
class XCodecParameters;

using XCodecParameters_sp = typename std::shared_ptr<XCodecParameters>;

struct XRational final {
    int num{1}, ///< Numerator
        den{1}; ///< Denominator
};

class XFormat {

    static int Time_out_callback(void *);
    void destroy();
public:
    /**
     * 设置AVFormatContext,线程安全,ctx传nullptr,代表要销毁当前上下文
     * @param ctx
     */
    void set_fmt_ctx(AVFormatContext *ctx);

    /**
     * 拷贝编码参数,线程安全
     * @param stream_index 流编号
     * @param dst 目标位置
     * @return ture or false
     */
    bool CopyParm(const int &stream_index,AVCodecParameters *dst);

    bool CopyParm(const int &stream_index,AVCodecContext *dst);

    XCodecParameters_sp CopyVideoParm() ;
    /**
     * 获取视频流index
     * @return index
     */
    [[nodiscard]] auto video_index() const{return m_video_index.load();}

    /**
     * 获取音频流index
     * @return index
     */
    [[nodiscard]] auto audio_index() const{return m_audio_index.load();}

    /**
     * 获取视频流时间基准
     * @return XRational
     */
    [[nodiscard]] auto video_timebase() const{return m_video_timebase;}

    /**
     * 获取音频流时间基准
     * @return XRational
     */
    [[nodiscard]] auto audio_timebase() const{return m_audio_timebase;}

    /**
     *重新计算pts dst duration
     * @param packet
     * @param offset_pts
     * @param time_base
     * @return ture or false
     */
    bool RescaleTime(XAVPacket &packet,const int64_t &offset_pts,const XRational &time_base);

    /**
     * 获取编码/解码器id
     * @return
     */
    [[nodiscard]] auto codec_id() const{return m_codec_id.load();}

    /**
     * 设置IO超时时间
     * @param ms
     * @return true or false
     */
    bool set_timeout_ms(const uint64_t &ms = 1000);

    /**
     * 检查是否超时
     * @return true or false
     */
    [[nodiscard]] bool IsTimeout();

    /**
     * 是否断开重连
     * @return
     */
    [[nodiscard]] auto is_connected(){return m_is_connected.load();}

protected:
    std::mutex m_mux;
    AVFormatContext *m_fmt_ctx{};
    std::atomic_bool m_is_connected{};
    std::atomic_int m_audio_index{-1},
        m_video_index{-1},
        m_codec_id{};

    std::atomic_uint64_t m_time_out_ms{},
        m_last_time{};

    XRational m_video_timebase{1,25},
                m_audio_timebase{1,44100};

protected:
    explicit XFormat() = default;
    virtual ~XFormat();
    X_DISABLE_COPY_MOVE(XFormat)
};

#endif
