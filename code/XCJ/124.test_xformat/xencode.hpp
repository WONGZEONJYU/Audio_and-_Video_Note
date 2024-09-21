//
// Created by Administrator on 2024/9/7.
//

#ifndef INC_116_TEST_XENCODE_XENCODE_HPP
#define INC_116_TEST_XENCODE_XENCODE_HPP

#include "xcodec.hpp"

using XAVPackets = typename std::vector<XAVPacket_sptr>;

class XEncode : public XCodec {

public:
    /**
     * 编码数据,线程安全,调用一次,创建一次AVPacket对象
     * AVPacket,AVFrame让智能指针自动维护,无需用户干预
     * @param frame
     * @return XAVPacket_sptr or nullptr
     */
    XAVPacket_sptr Encode(const XAVFrame *frame);

    /**
     * 冲刷编码器
     * @return XAVPackets or empty
     */
    XAVPackets Flush();

public:
    explicit XEncode() = default;
    ~XEncode() override = default;
    X_DISABLE_COPY_MOVE(XEncode)
};

#endif
