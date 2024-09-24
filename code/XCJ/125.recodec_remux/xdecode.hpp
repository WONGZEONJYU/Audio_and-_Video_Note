//
// Created by Administrator on 2024/9/10.
//

#ifndef INC_120_TEST_XDECODE_XDECODE_HPP
#define INC_120_TEST_XDECODE_XDECODE_HPP

#include "xcodec.hpp"

using XAVFrames = typename std::vector<XAVFrame_sptr>;

class XDecode : public XCodec {

public:
    /**
     * 发送包到解码器
     * @param packet
     * @return true or false
     */
    bool Send(const XAVPacket &packet);

    /**
     * 从解码器接收解码后到帧,需循环读取
     * @param frame
     * @return true or false
     */
    bool Receive(XAVFrame &frame);

    /**
     * 冲刷解码器,把缓冲的帧全部读取出来
     * @return XAVFrames or empty
     */
    XAVFrames Flush();

    /**
     * 初始化硬解码
     * @param type
     * @return
     */
    bool InitHw(const int &type = 4);

public:
    explicit XDecode() = default;
    ~XDecode() override = default;
    X_DISABLE_COPY_MOVE(XDecode)
};

#endif
