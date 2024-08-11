//
// Created by Administrator on 2024/7/31.
//

#ifndef XPLAY2_XDECODE_HPP
#define XPLAY2_XDECODE_HPP

#include <mutex>
#include <memory>
#include "XHelper.hpp"

class XAVCodecParameters;
class XAVFrame;
class XAVPacket;
struct AVCodecContext;

using XAVCodecParameters_sptr = typename std::shared_ptr<XAVCodecParameters>;
using XAVFrame_sptr = typename std::shared_ptr<XAVFrame>;
using XAVPacket_sptr = typename std::shared_ptr<XAVPacket>;

class XDecode {
    void DeConstruct() noexcept(true);
public:
    explicit XDecode() = default;
    /**
     * 打开解码器,参数用音频或者视频解码参数
     */
    virtual void Open(const XAVCodecParameters_sptr &) noexcept(false);
    /**
     * 发送解码,发送空packet冲刷解码器,有异常
     * @return
     */
    virtual bool Send(const XAVPacket_sptr &) noexcept(false);
    /**
     * 接收解码后的帧,需判空,有异常
     * @return
     */
    virtual XAVFrame_sptr Receive() noexcept(false);
    /**
     * 关闭解码器
     */
    virtual void Close() noexcept(false);
    /**
     * 清空解码器
     */
    virtual void Clear() noexcept(false);

    /**
     * @return 解码后的pts
     */
    [[nodiscard]] int64_t Pts() const noexcept(true) {return m_pts;}

protected:
    std::recursive_mutex m_re_mux;
    AVCodecContext *m_codec_ctx{};
    std::atomic_int64_t m_pts{};
public:
    virtual ~XDecode();
    X_DISABLE_COPY(XDecode);
};

#endif
