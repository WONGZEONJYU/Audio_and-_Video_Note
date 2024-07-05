//
// Created by Administrator on 2024/4/19.
//

#ifndef INC_15_MP4_MUXER_SHAREAVPACKET_HPP
#define INC_15_MP4_MUXER_SHAREAVPACKET_HPP

struct AVPacket;
#include <memory>

struct ShareAVPacket final {

    using ShareAVPacket_sp_type = std::shared_ptr<ShareAVPacket>;
    ShareAVPacket(const ShareAVPacket&) = delete;
    ShareAVPacket& operator=(const ShareAVPacket&) = delete;
    static ShareAVPacket_sp_type create() noexcept(false);
    ~ShareAVPacket();
    operator AVPacket* () const {return m_packet;}
    AVPacket *operator->() const {return m_packet;}
    AVPacket operator*() const {return *m_packet;}
private:
    AVPacket *m_packet{};
    ShareAVPacket() noexcept(true);
    void Construct() const noexcept(false);
    void DeConstruct() noexcept(true);
};

using ShareAVPacket_sp_type = typename ShareAVPacket::ShareAVPacket_sp_type;

ShareAVPacket_sp_type new_ShareAVPacket() noexcept(false);

#endif
