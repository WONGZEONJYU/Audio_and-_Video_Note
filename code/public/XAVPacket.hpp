//
// Created by Administrator on 2024/7/29.
//

#ifndef XPLAY2_XAVPACKET_HPP
#define XPLAY2_XAVPACKET_HPP

extern "C"{
#include <libavcodec/packet.h>
}
#include <memory>

class XAVPacket final : public AVPacket{
public:
    XAVPacket();
    /**
     * 引用计数+1
     */
    XAVPacket(const XAVPacket &);
    XAVPacket(XAVPacket &&) noexcept;
    /**
     * 引用计数+1
     * @return XAVPacket&
     */
    XAVPacket& operator=(const XAVPacket &);
    XAVPacket& operator=(XAVPacket &&) noexcept;
    ~XAVPacket();

    /**
     * 为给定数据包描述的数据创建可写引用,尽可能避免数据复制
     * @return true or false
     */
    bool Make_Writable();

};

using XAVPacket_sptr = typename std::shared_ptr<XAVPacket>;
XAVPacket_sptr new_XAVPacket() noexcept(false);

#endif
