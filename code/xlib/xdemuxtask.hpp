#ifndef XDEMUXTASK_HPP_
#define XDEMUXTASK_HPP_

#include "xavframe.hpp"
#include "xtools.hpp"
#include "xdemux.hpp"

enum SYNC_TYPE {
    NONE_SYNC = 0,
    SYNC_VIDEO,
};

class XLIB_API XDemuxTask final: public XThread {

    void Main() override;
public:
    explicit XDemuxTask() = default;
    ~XDemuxTask() override;

    bool Open(const std::string &url,const uint64_t &time_out = 1000);
    void Stop() override;
    void Clear();
    auto CopyVideoParm() const{return m_demux_.CopyVideoParm();}
    auto CopyAudioParm() const{return m_demux_.CopyAudioParm();}
    auto audio_index() const{return m_demux_.audio_index();}
    auto video_index() const{return m_demux_.video_index();}
    /**
     * 如果用于播放器,没有音频的时候,需视频自己进行同步
     * @param type
     */
    void set_sync_type(const SYNC_TYPE &type){m_sync_type_ = type;}

    /**
     * 没有音频只有视频的倍速
     * @param speed
     */
    inline void set_speed(const double &speed){
        m_speed_ = speed < 0.1 ? 0.1 : speed;
    }

    [[nodiscard]] bool Seek(const int64_t &ms);

    [[maybe_unused]] [[nodiscard]] bool ReadVideoPacket(XAVPacket &pkt);

    [[maybe_unused]]inline bool is_Open() const {
        return m_is_open_;
    }

    inline explicit operator bool() const {
        return m_is_open_;
    }

    inline bool operator!() const {
        return !m_is_open_;
    }

private:
    XDemux m_demux_;
    std::string m_url_;
    std::atomic_uint_fast64_t m_timeout_ms_{};
    std::atomic<SYNC_TYPE> m_sync_type_{NONE_SYNC};
    std::atomic<double> m_speed_{1.0};
    std::atomic_bool m_is_open_{};
    X_DISABLE_COPY_MOVE(XDemuxTask)
};

#endif
