//
// Created by Administrator on 2024/7/29.
//

#ifndef XPLAY2_XAVPACKET_HPP
#define XPLAY2_XAVPACKET_HPP

extern "C"{
#include <libavcodec/packet.h>
}
#include <memory>

class XAVPacket final : public AVPacket {
public:
    XAVPacket();
    /**
     * 把AVPacket对象(并非AVPacket对象本身)成员变量移动到XAVPacket进行管理
     * 如果AVPacket对象是通过av_packet_alloc()申请的,移动后,需用户手动调用av_packet_free()清理
     * 不会对移动后到数据产生任何影响
     * @param packet
     */
    explicit XAVPacket(AVPacket &packet);

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

    /**
     *
     * @param packet
     */
    void Reset(AVPacket *packet = nullptr);

};

using XAVPacket_sptr = typename std::shared_ptr<XAVPacket>;
XAVPacket_sptr new_XAVPacket() noexcept(false);

#endif
