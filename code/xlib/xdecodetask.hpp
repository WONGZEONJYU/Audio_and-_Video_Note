#ifndef XDECODETASK_HPP_
#define XDECODETASK_HPP_

#include "xtools.hpp"
#include "xdecode.hpp"

class XLIB_API XDecodeTask final: public XThread {

    void Main() override;

public:
    void Do(XAVPacket &) override;
    /**
     * 打开解码器
     * @param parm
     * @return true or false
     */
    bool Open(const XCodecParameters_sp &parm);
    bool Open(const XCodecParameters &parm);
    void Stop() override;
    void Clear();
    /**
     * 拷贝解码后的一帧数据,有可能为空
     * @return XAVFrame_sp
     */
    XAVFrame_sp CopyFrame();

    /**
     * 区分音频还是视频
     * @param index
     */
    inline void set_stream_index(const int &index) {
        m_stream_index_ = index;
    }

    /**
     * 此函数为帧缓存策略
     * 对于视频来说,这里设置不设置无所谓,丢帧不容易发现
     * 对于音频来说,丢帧非常容易发现,如发现音频丢帧,设置这里采用缓存策略可以避免这个问题
     * 当然视频也可以采用帧缓存策略
     * @param b
     */
    inline void set_frame_cache(const bool &b) {
        m_frame_cache_ = b;
    }

    inline bool is_open() const {
        return m_is_open_;
    }

    inline explicit operator bool() const {
        return m_is_open_;
    }

    inline bool operator!() const {
        return !m_is_open_;
    }

    /**
     * 设置同步
     * @param sync_pts
     */
    inline void set_sync_pts(const int64_t &sync_pts) {
        m_sync_pts_ = sync_pts;
    }

    /**
     * 设置AVPacket_list阻塞大小
     * @param s
     */
    inline void set_block_size(const int &s){
        m_block_size = s;
    }

    /**
     * 返回当前帧的pts转换成pts_ms
     * @return pts_ms
     */
    auto curr_pts_ms() const {
        return m_curr_ms_.load(std::memory_order_relaxed);
    }

    /**
     * 返回当前帧的pts
     * @return pts
     */
    auto curr_pts() const {
        return m_curr_pts.load(std::memory_order_relaxed);
    }

    /**
     * 一个辅助解码,可用于正常解码和Seek操作后单独解码显示
     * 需要显示的话,调用CopyFrame
     * @return pkt
     */
    bool Decode(const XAVPacket &pkt);

private:
    std::mutex m_mutex_;
    XDecode m_decode_;
    XAVPacketList m_pkt_list_;
    std::list<XAVFrame_sp> m_frames_;
    XAVFrame_sp m_frame_;
    std::atomic_bool m_need_view_{},
                    m_frame_cache_{},
                    m_is_open_{};
    std::atomic_int_fast32_t m_stream_index_{-1},
                    m_block_size{-1};
    std::atomic_int_fast64_t m_sync_pts_{-1},
                            m_curr_ms_{-1},
                            m_curr_pts{-1};
    XCodecParameters_sp m_paras_;
public:
    explicit XDecodeTask() = default;
    ~XDecodeTask() override;
    X_DISABLE_COPY_MOVE(XDecodeTask)
};

#endif
