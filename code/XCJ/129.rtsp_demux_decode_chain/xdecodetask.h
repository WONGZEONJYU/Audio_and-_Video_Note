#ifndef XDECODETASK_H
#define XDECODETASK_H

#include "xtools.hpp"
#include "xdecode.hpp"

class XDecodeTask : public XThread{

    void Do(XAVPacket &) override;
    void Main() override;
public:
    /**
     * 打开解码器
     * @param parm
     * @return true or false
     */
    bool Open(const XCodecParameters_sp &parm);

    XAVFrame_sp CopyFrame();

private:
    XAVPacketList m_pkt_list_;
    XDecode m_decode_;
    std::mutex m_mutex_;
    XAVFrame_sp m_frame_;
    std::atomic<bool> m_need_view_;
public:
    explicit XDecodeTask() = default;
    X_DISABLE_COPY_MOVE(XDecodeTask)
};

#endif
