//
// Created by Fy-WONG on 2024/10/11.
//

#ifndef XMUXTASK_H
#define XMUXTASK_H

#include "xtools.hpp"
#include "xmux.hpp"

class XMuxTask : public XThread {
    void Main() override;
public:
    bool Open(const std::string &url,
        const XCodecParameters *video_parm = nullptr,
        const AVRational *video_tb = nullptr,
        const XCodecParameters *audio_parm = nullptr,
        const AVRational *audio_tb = nullptr);

private:
    std::mutex m_mux_;
    XMux m_xmux_;

public:
    explicit  XMuxTask() = default;
    ~XMuxTask() override = default;
    X_DISABLE_COPY_MOVE(XMuxTask)
};

#endif //XMUXTASK_H
