#ifndef XMUX_HPP_
#define XMUX_HPP_

#include "xformat.hpp"

class XLIB_API XMux final : public XFormat{

    [[maybe_unused]] void destroy();

public:
    /**
     * 创建并打开复用器
     * @param url
     * @param video_parm
     * @param audio_parm
     * @return AVFormatContext* or nullptr
     */
    static AVFormatContext *Open(const std::string &url,
                                 const XCodecParameters& video_parm,
                                 const XCodecParameters& audio_parm);

    /**
     * 创建并打开复用器,指针可以为nullptr
     * @param url
     * @param video_parm
     * @param audio_parm
     * @return AVFormatContext* or nullptr
     */
    static AVFormatContext *Open(const std::string &url,
                                 const XCodecParameters* video_parm = {},
                                 const XCodecParameters* audio_parm = {});

    /**
     * 音视频时间基准参数
     * @param tb
     */
    void set_video_time_base(const AVRational &tb);
    void set_audio_time_base(const AVRational &tb);

    /**
     * 写入头部信息
     * @return ture or false
     */
    bool WriteHead();

    /**
     * 写入包数据
     * @param packet
     * @return ture or false
     */
    bool Write(XAVPacket &packet);

    /**
     * 写入尾部信息
     * @return ture or false
     */
    bool WriteEnd();

private:
#if 1
    XRational m_src_video_time_base_,
            m_src_audio_time_base_;
#else
    AVRational *m_src_video_time_base_{},
                *m_src_audio_time_base_{};
#endif
    std::atomic_int64_t m_src_begin_video_pts_{-1},
            m_src_begin_audio_pts_{-1};
public:
    explicit XMux() = default;
    ~XMux() override;
    X_DISABLE_COPY_MOVE(XMux)
};

#endif
