//
// Created by Administrator on 2024/9/7.
//

#ifndef INC_116_TEST_XENCODE_XENCODE_HPP
#define INC_116_TEST_XENCODE_XENCODE_HPP

#include "XHelper.hpp"
#include <mutex>
#include <string>

struct AVCodecContext;

class XEncode {

public:
    /**
     * 创建编码上下文
     * @param codec_id 对应ffmpeg
     * @return AVCodecContext* or nullptr
     */
    static AVCodecContext *Create(const int &codec_id);

    /**
     * 设置编码器上下文 上下文传递到对象中,资源由XEncode维护
     * 加锁 线程安全
     * 如果m_codec_ctx不为nullptr,则先清理资源
     * @param ctx
     */
    void set_codec_ctx(AVCodecContext *ctx);

    /**
     * 设置编码器参数,线程安全
     * @param key
     * @param val
     */
    void SetOpt(const std::string &key,const std::string &val);
    void SetOpt(const std::string &key,const int64_t &val);

private:
    AVCodecContext *m_codec_ctx{};
    std::mutex m_mux;

public:
    explicit XEncode() = default;
    virtual ~XEncode() = default;
    X_DISABLE_COPY_MOVE(XEncode)
};

#endif
