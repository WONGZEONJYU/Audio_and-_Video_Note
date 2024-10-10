//
// Created by Administrator on 2024/7/29.
//

#ifndef XPLAY2_XAVPACKET_HPP
#define XPLAY2_XAVPACKET_HPP

extern "C"{
#include <libavcodec/packet.h>
}
#include <memory>
#include "xhelper.hpp"

class XAVPacket final : public AVPacket {
public:
    XAVPacket();

    /**
     * 对AVPacket拷贝,引用计数+1,AVPacket对象本身由用户自行管理
     * @param packet
     */
    explicit XAVPacket(const AVPacket &packet);

    /**
     * 同上
     * @param packet
     */
    explicit XAVPacket(const AVPacket *packet);

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
     * 释放对象本身数据,如果packet不为nullptr
     * 则对AVPacket引用计数+1
     * @param packet
     */
    void Reset(const AVPacket *packet = nullptr);

    /**
     * 对AVPacket进行引用计数+1,如果packet == null,则不做任何操作
     * @param packet
     */
    void Ref_fromAVPacket(const AVPacket *packet);
    void Ref_fromAVPacket(const AVPacket &packet);

    /**
     * 转移AVPacket的引用计数,被转移后的AVPacket谨慎使用
     * packet == null,不做任何动作
     * @param packet
     */
    void Move_FromAVPacket(AVPacket *packet);
    void Move_FromAVPacket(AVPacket &&packet);
};

XAVPacket_sp new_XAVPacket() noexcept(true);
XAVPacket_sp new_XAVPacket(const AVPacket &packet) noexcept(true);
XAVPacket_sp new_XAVPacket(const AVPacket *packet) noexcept(true);

#endif
