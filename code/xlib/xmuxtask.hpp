#ifndef XMUXTASK_H
#define XMUXTASK_H

#include "xtools.hpp"
#include "xmux.hpp"

class XLIB_API XMuxTask final : public XThread {
    void Main() override;
    void Do(XAVPacket &) override;
public:
    bool Open(const std::string &url,
        const XCodecParameters &video_parm ,
        const XCodecParameters &audio_parm);

    bool Open(const std::string &url,
        const XCodecParameters* video_parm = {},
        const XCodecParameters* audio_parm = {});

private:
    std::mutex m_mux_;
    XMux m_xmux_;
    XAVPacketList m_pkts_;
public:
    explicit XMuxTask() = default;
    ~XMuxTask() override ;
    X_DISABLE_COPY_MOVE(XMuxTask)
};

#endif
