//
// Created by Administrator on 2024/9/10.
//

#ifndef INC_120_TEST_XDECODE_XDECODE_HPP
#define INC_120_TEST_XDECODE_XDECODE_HPP

#include "xcodec.hpp"

using XAVFrames = typename std::vector<XAVFrame_sptr>;

class XDecode : public XCodec {

public:
    bool Send(const XAVPacket_sptr &packet);
    bool Receive(XAVFrame_sptr &frame);
    XAVFrames Flush();
    explicit XDecode() = default;
    ~XDecode() override = default;
    X_DISABLE_COPY_MOVE(XDecode)
};

#endif
