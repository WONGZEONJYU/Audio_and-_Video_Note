//
// Created by Fy-WONG on 2024/10/8.
//

#ifndef XDECODETASK_H
#define XDECODETASK_H

#include <list>
#include "xtools.hpp"
#include "xdecode.hpp"

class XDecodeTask : public XThread{

    void Main() override;
    void Do(XAVPacket &) override;
public:
    /**
     * 打开编码器
     * @param parms
     * @return true or false
     */
    bool Open(const XCodecParameters &parms);

    XAVFrame_sp CopyFrame();

private:
    XAVPacketList m_pkt_list;
    XDecode m_decode;
    std::mutex m_mutex;
    XAVFrame_sp m_frame;
    std::atomic<bool> m_need_view_;
public:
    explicit XDecodeTask() = default;
    X_DISABLE_COPY_MOVE(XDecodeTask)
};

#endif
