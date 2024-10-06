//
// Created by wong on 2024/9/27.
//

#ifndef INC_126_RTSP_DEMUX_THREAD_XDEMUXTASK_HPP
#define INC_126_RTSP_DEMUX_THREAD_XDEMUXTASK_HPP

#include "xtools.hpp"
#include "xdemux.hpp"

class XDemuxTask : public XThread{

    void Main() override;
public:
    bool Open(const std::string &url,const uint64_t &time_out = 1000);

private:
    XDemux m_demux;
    std::string m_url;
    uint64_t m_timeout_ms{};
public:
    explicit XDemuxTask() = default;
    X_DISABLE_COPY_MOVE(XDemuxTask)
};

#endif //INC_126_RTSP_DEMUX_THREAD_XDEMUXTASK_HPP
