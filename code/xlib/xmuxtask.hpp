//
// Created by Fy-WONG on 2024/10/11.
//

#ifndef XMUXTASK_H
#define XMUXTASK_H

#include "xtools.hpp"
#include "xmux.hpp"

class XMuxTask : public XThread {
    void Main() override;
    void Do(XAVPacket &) override;
public:
    bool Open(const std::string &url,
        const XCodecParameters_sp &video_parm = {},
        const XCodecParameters_sp &audio_parm = {});

private:
    std::mutex m_mux_;
    XMux m_xmux_;
    XAVPacketList m_pkts_;

public:
    explicit  XMuxTask() = default;
    ~XMuxTask() override = default;
    X_DISABLE_COPY_MOVE(XMuxTask)
};

#endif //XMUXTASK_H
