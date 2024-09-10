//
// Created by Administrator on 2024/9/7.
//

#ifndef INC_116_TEST_XENCODE_XENCODE_HPP
#define INC_116_TEST_XENCODE_XENCODE_HPP

#include "XHelper.hpp"
#include <mutex>
#include <string>
#include <vector>
#include <memory>
#include "encode_params.hpp"

struct AVCodecContext;
class XAVFrame;
class XAVPacket;

using XAVFrame_sptr = typename std::shared_ptr<XAVFrame>;
using XAVPacket_sptr = typename std::shared_ptr<XAVPacket>;

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
     * @return ture or false
     */

    bool SetOpt(const std::string &key,const std::string &val);
    bool SetOpt(const std::string &key,const int64_t &val);

    /**
     * 设置Preset参数,线程安全
     * @tparam T
     * @param p
     * @return ture or false
     */
    template<typename T>
    std::enable_if_t<std::is_base_of_v<PRESET,T>,bool> Set_Preset(const T &p){
        return SetOpt(p.first,p.second);
    }

    /**
     * 设置Tune参数,线程安全
     * @tparam T
     * @param t
     * @return ture or false
     */
    template<typename T>
    std::enable_if_t<std::is_base_of_v<TUNE,T>,bool > Set_Tune(const T &t){
        return SetOpt(t.first,t.second);
    }

    /**
     * 设置Profile参数,线程安全
     * @tparam T
     * @param p
     * @return ture or false
     */
    template<typename T>
    std::enable_if_t<std::is_base_of_v<PROFILE,T>,bool> Set_Profile(const T &p){
        return SetOpt(p.first,p.second);
    }

    /**
     * 设置qp值
     * @param qp
     * @return ture or false
     */
    bool Set_Qp(const QP &qp);

    /**
     * 设置NAL_HRD,线程安全
     * @tparam T
     * @param n
     * @return ture or false
     */
    template<typename T>
    std::enable_if_t<std::is_base_of_v<NAL_HRD,T>,bool > Set_NAL_HRD(const T &n){
        return SetOpt(n.first,n.second);
    }

    /**
     * 设置crf,线程安全
     * @param crf
     * @return ture or false
     */
    bool Set_CRF(const CRF &crf);

    /**
     * 打开编码器,线程安全
     * @return ture or false
     */
    bool Open();

    /**
     * 编码数据,线程安全,调用一次,创建一次AVPacket对象
     * AVPacket,AVFrame让智能指针自动维护,无需用户干预
     * @param frame
     * @return XAVPacket_sptr or nullptr
     */
    XAVPacket_sptr Encode(const XAVFrame_sptr &frame);

    /**
     * 冲刷编码器
     * @return std::vector<XAVPacket_sptr>
     */
    std::vector<XAVPacket_sptr> Flush();

    /**
     * 分配AVFrame对象
     * @return XAVFrame_sptr or nullptr
     */
    XAVFrame_sptr Alloc_AVFrame();

private:
    AVCodecContext *m_codec_ctx{};
    std::mutex m_mux;

public:
    explicit XEncode() = default;
    virtual ~XEncode();
    X_DISABLE_COPY_MOVE(XEncode)
};

#endif
