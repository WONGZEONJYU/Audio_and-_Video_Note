#ifndef XCODEC_HPP_
#define XCODEC_HPP_

#include "encode_params.hpp"
#include "xhelper.hpp"

class XLIB_API XCodec {

    void destroy();
public:
    /**
     * 创建编解码上下文
     * @param codec_id 对应ffmpeg
     * @param is_encode
     * @return AVCodecContext* or nullptr
     */
    static AVCodecContext *Create(const int &codec_id,const bool &is_encode);

    /**
     * 设置编解码器上下文传递到对象中,资源由XCodec维护
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

    [[nodiscard]] bool SetOpt(const std::string &key,const std::string &val) const;
    [[nodiscard]] bool SetOpt(const std::string &key,const int64_t &val) const;

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
    [[nodiscard]] bool Set_Qp(const QP &qp) const {
       return SetOpt(QP::m_name,qp.value());
    }

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
    [[nodiscard]] bool Set_CRF(const CRF &crf) const {
        return SetOpt(CRF::m_name,crf.value());
    }

    /**
     * 打开编码器,线程安全
     * @return ture or false
     */
    [[nodiscard]] bool Open();

    /**
     * 获取一个已经分配好符合当前音视频帧空间的一个frame
     * @param align 对齐
     * @param
     * @return
     */
    [[nodiscard]] [[maybe_unused]] XAVFrame_sp CreateFrame(const int &align) const;

    virtual void Clear();

protected:
    std::mutex m_mux_;
    AVCodecContext *m_codec_ctx_{};
    explicit XCodec() = default;
    virtual ~XCodec();
    X_DISABLE_COPY_MOVE(XCodec)

#define CHECK_CODEC_CTX_RET() \
std::unique_lock locker(const_cast<decltype(m_mux_)&>(m_mux_));\
do{if (!m_codec_ctx_){\
PRINT_ERR_TIPS(GET_STR(AVCodecContext Not Created!));\
return {};}}while(false)

#define CHECK_CODEC_CTX() \
std::unique_lock locker(const_cast<decltype(m_mux_)&>(m_mux_));\
do{if (!m_codec_ctx_){\
PRINT_ERR_TIPS(GET_STR(AVCodecContext Not Created!));\
return;}}while(false)

#define CHECK_ENCODE_OPEN() do{ \
if (avcodec_is_open(m_codec_ctx_)){\
PRINT_ERR_TIPS(GET_STR((encode is open,Invalid parameter setting\n)));} \
}while(false)

};

#endif
