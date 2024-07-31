//
// Created by Administrator on 2024/7/31.
//

#ifndef XPLAY2_XDECODE_HPP
#define XPLAY2_XDECODE_HPP

#include <mutex>
#include <memory>
#include "XHelper.h"

class XAVCodecParameters;
class XAVFrame;
class XAVPacket;
using XAVCodecParameters_sptr = typename std::shared_ptr<XAVCodecParameters>;
using XAVFrame_sptr = typename std::shared_ptr<XAVFrame>;
using XAVPacket_sptr = typename std::shared_ptr<XAVPacket>;

class XDecode {
    void DeConstruct() noexcept(true);
public:
    explicit XDecode() = default;
    virtual void Open(const XAVCodecParameters_sptr &) noexcept(false);
    [[maybe_unused]] [[nodiscard]] virtual bool Send(const XAVPacket_sptr &) const noexcept(false);

    [[maybe_unused]] [[nodiscard]] virtual XAVFrame_sptr Receive() const noexcept(false);
    virtual void Close() noexcept(false);
    virtual void Clear() noexcept(false);
protected:
    std::recursive_mutex m_re_mux;
    AVCodecContext *m_codec_ctx{};

public:
    virtual ~XDecode();
    X_DISABLE_COPY(XDecode);
};

#endif
