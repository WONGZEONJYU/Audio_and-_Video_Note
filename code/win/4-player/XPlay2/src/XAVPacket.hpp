//
// Created by Administrator on 2024/7/29.
//

#ifndef XPLAY2_XAVPACKET_HPP
#define XPLAY2_XAVPACKET_HPP

extern "C"{
#include <libavcodec/packet.h>
}
#include <memory>

struct XAVPacket final : public AVPacket{

    XAVPacket();
    XAVPacket(const XAVPacket &);
    XAVPacket(XAVPacket &&) noexcept;
    XAVPacket& operator=(const XAVPacket &);
    XAVPacket& operator=(XAVPacket &&) noexcept;
    ~XAVPacket();
};

using XAVPacket_sptr = std::shared_ptr<XAVPacket>;
XAVPacket_sptr new_XAVPacket() noexcept(false);

#endif
