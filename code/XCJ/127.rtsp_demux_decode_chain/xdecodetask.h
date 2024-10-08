//
// Created by Fy-WONG on 2024/10/8.
//

#ifndef XDECODETASK_H
#define XDECODETASK_H

#include <list>
#include <memory>
#include "xtools.hpp"
#include "xdecode.hpp"

class XAVPacket;
class XCodecParameters;

using XAVPacket_sptr = std::shared_ptr<XAVPacket>;

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

private:
    std::list<XAVPacket_sptr> m_packets;
    XDecode m_decode;
    std::mutex m_mutex;

public:
    explicit XDecodeTask() = default;
    X_DISABLE_COPY_MOVE(XDecodeTask)

};

#endif
