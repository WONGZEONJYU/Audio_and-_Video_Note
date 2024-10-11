//
// Created by Fy-WONG on 2024/10/11.
//

#ifndef XMUXTASK_H
#define XMUXTASK_H

#include "xtools.hpp"

class XMuxTask : public XThread {
    void Main() override;
public:
    bool Open(const std::string &url,
        const XCodecParameters &video_parm,
        const AVRational &video_tb);

private:

public:
    explicit  XMuxTask() = default;
    ~XMuxTask() override = default;
    X_DISABLE_COPY_MOVE(XMuxTask)
};

#endif //XMUXTASK_H
