#ifndef XAVPACKET_HPP_
#define XAVPACKET_HPP_

extern "C"{
#include <libavcodec/packet.h>
}

#include "xhelper.hpp"

class XLIB_API XAVPacket final : public AVPacket {
public:
    XAVPacket();

    /**
     *
     * @param packet
     */
    explicit XAVPacket(const AVPacket &packet);

    /**
     * ??
     * @param packet
     */
    explicit XAVPacket(const AVPacket *packet);

    /**
     * ????+1
     */
    XAVPacket(const XAVPacket &);
    XAVPacket(XAVPacket &&) noexcept;
    /**
     * ????+1
     * @return XAVPacket&
     */
    XAVPacket& operator=(const XAVPacket &);
    XAVPacket& operator=(XAVPacket &&) noexcept;
    ~XAVPacket();

    /**
     * ?????????????????,?????????
     * @return true or false
     */
    bool Make_Writable();

    /**
     * ????????,??packet??nullptr
     * ??AVPacket????+1
     * @param packet
     */
    void Reset(const AVPacket *packet = nullptr);

    /**
     * ?AVPacket??????+1,??packet == null,???????
     * @param packet
     */
    void Ref_fromAVPacket(const AVPacket *packet);
    void Ref_fromAVPacket(const AVPacket &packet);

    /**
     * ??AVPacket?????,?????AVPacket????
     * packet == null,??????
     * @param packet
     */
    void Move_FromAVPacket(AVPacket *packet);
    void Move_FromAVPacket(AVPacket &&packet);

    explicit operator bool() const;
    bool operator !() const;
    [[nodiscard]] bool empty() const;
};

XLIB_API XAVPacket_sp new_XAVPacket() noexcept(true);
XLIB_API XAVPacket_sp new_XAVPacket(const AVPacket &packet) noexcept(true);
XLIB_API XAVPacket_sp new_XAVPacket(const AVPacket *packet) noexcept(true);

#endif
