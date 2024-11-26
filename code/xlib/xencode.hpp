#ifndef XENCODEC_HPP_
#define XENCODEC_HPP_

#include "xcodec.hpp"

using XAVPackets = std::vector<XAVPacket_sp>;

class XLIB_API XEncode final : public XCodec {

public:
    /**
     * 编码数据,线程安全,调用一次,创建一次AVPacket对象
     * AVPacket,AVFrame让智能指针自动维护,无需用户干预
     * @param frame
     * @return XAVPacket_sptr or nullptr
     */
    [[nodiscard]] XAVPacket_sp Encode(const XAVFrame &frame) const;

    /**
     * 冲刷编码器
     * @return XAVPackets or empty
     */
    [[nodiscard]] XAVPackets Flush() const;

    explicit XEncode() = default;
    ~XEncode() override = default;
    X_DISABLE_COPY_MOVE(XEncode)
};

#endif
