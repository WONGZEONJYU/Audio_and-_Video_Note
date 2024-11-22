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

private:
    XAVPacketList m_pkt_list_;
    XDecode m_decode_;
    std::mutex m_mutex_;
    XAVFrame_sp m_frame_;
    std::atomic_bool m_need_view_{},
        m_frame_cache_{},m_is_open_{};
    std::atomic_int m_stream_index_{};
    std::list<XAVFrame_sp> m_frames_;
    std::atomic_int64_t m_sync_pts_{-1};
public:
    explicit XDecodeTask() = default;
    ~XDecodeTask() override;
    X_DISABLE_COPY_MOVE(XDecodeTask)
};

#endif
