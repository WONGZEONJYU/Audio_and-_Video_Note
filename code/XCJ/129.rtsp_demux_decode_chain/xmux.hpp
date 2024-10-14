//
// Created by wong on 2024/9/22.
//

#ifndef INC_124_TEST_XFORMAT_XMUX_HPP
#define INC_124_TEST_XFORMAT_XMUX_HPP

#include "xformat.hpp"

class XMux : public XFormat{

    void destroy();

public:
    static AVFormatContext *Open(const std::string &url,
                                 const XCodecParameters_sp &video_parm = {},
                                 const XCodecParameters_sp &audio_parm = {});

    /**
     * 音视频时间基准参数
     * @param tb
     */
    void set_video_time_base(const AVRational *tb);
    void set_audio_time_base(const AVRational *tb);

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
    AVRational *m_src_video_time_base_{},
                *m_src_audio_time_base_{};

public:
    explicit XMux() = default;
    ~XMux() override;
    X_DISABLE_COPY_MOVE(XMux)
};

#endif
