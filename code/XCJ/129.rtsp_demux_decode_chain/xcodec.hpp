#ifndef INC_120_TEST_XDECODE_XCODEC_HPP
#define INC_120_TEST_XDECODE_XCODEC_HPP

#include "encode_params.hpp"
#include "xhelper.hpp"

class XCodec {

    void destroy();
public:
    /**
     * 创建编解码上下文
     * @param codec_id 对应ffmpeg
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
     * 分配AVFrame对象
     * @param one_frame_size 获取分配大小
     * @return
     */
    [[nodiscard]] XAVFrame_sp CreateFrame() const;

protected:
    std::mutex m_mux_;
    AVCodecContext *m_codec_ctx_{};
    explicit XCodec() = default;
    virtual ~XCodec();
    X_DISABLE_COPY_MOVE(XCodec)


#define CHECK_CODEC_CTX() \
std::unique_lock locker(const_cast<decltype(m_mux_)&>(m_mux_));\
do{if (!m_codec_ctx_){\
PRINT_ERR_TIPS(GET_STR(AVCodecContext Not Created!));\
return {};}}while(false)

};

#endif
