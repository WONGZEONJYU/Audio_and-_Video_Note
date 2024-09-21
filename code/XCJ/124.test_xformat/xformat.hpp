//
// Created by wong on 2024/9/21.
//

#ifndef INC_124_TEST_XFORMAT_XFORMAT_HPP
#define INC_124_TEST_XFORMAT_XFORMAT_HPP

#include "xhelper.hpp"
#include <mutex>
#include <atomic>

struct AVFormatContext;
struct AVCodecParameters;

struct XRational {
    int num; ///< Numerator
    int den; ///< Denominator
};

class XFormat {

    void destroy();
public:
    /**
     * 设置AVFormatContext,线程安全,ctx传nullptr,代表要销毁当前上下文
     * @param ctx
     */
    void set_fmt_ctx(AVFormatContext * ctx);

    /**
     * 拷贝编码参数,线程安全
     * @param stream_index 流编号
     * @param dst 目标位置
     * @return ture or false
     */
    bool CopyParm(const int &stream_index,AVCodecParameters *dst);

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
    [[nodiscard]] auto video_timebase() const{return m_video_timebase.load();}

    /**
     * 获取音频流时间基准
     * @return XRational
     */
    [[nodiscard]] auto audio_timebase() const{return m_audio_timebase.load();}

protected:
    std::mutex m_mux;
    AVFormatContext *m_fmt_ctx{};
    std::atomic_int m_audio_index{-1},
        m_video_index{-1};

    std::atomic<XRational> m_video_timebase,
        m_audio_timebase;

protected:
    explicit XFormat();

public:
    virtual ~XFormat();
    X_DISABLE_COPY_MOVE(XFormat)
};

#endif
