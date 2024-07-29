//
// Created by Administrator on 2024/4/19.
//

#ifndef INC_15_MP4_MUXER_SHAREAVPACKET_HPP
#define INC_15_MP4_MUXER_SHAREAVPACKET_HPP

struct AVPacket;
#include <memory>

struct XShareAVPacket final {

    using ShareAVPacket_sp_type = std::shared_ptr<XShareAVPacket>;
    XShareAVPacket(const XShareAVPacket&) = delete;
    XShareAVPacket& operator=(const XShareAVPacket&) = delete;
    static ShareAVPacket_sp_type create() noexcept(false);
    ~XShareAVPacket();
    operator AVPacket* () const {return m_packet;}
    AVPacket *operator->() const {return m_packet;}
    AVPacket operator*() const {return *m_packet;}
private:
    AVPacket *m_packet{};
    XShareAVPacket() noexcept(true);
    void Construct() const noexcept(false);
    void DeConstruct() noexcept(true);
};

using ShareAVPacket_sp_type = typename XShareAVPacket::ShareAVPacket_sp_type;

ShareAVPacket_sp_type new_ShareAVPacket() noexcept(false);

#endif
