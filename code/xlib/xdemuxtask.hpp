#ifndef XDEMUXTASK_HPP_
#define XDEMUXTASK_HPP_

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
    auto CopyVideoParm() const{return m_demux_.CopyVideoParm();}
    auto CopyAudioParm() const{return m_demux_.CopyAudioParm();}
    void set_sync_type(const SYNC_TYPE &type){m_sync_type_ = type;}
private:
    XDemux m_demux_;
    std::string m_url_;
    uint64_t m_timeout_ms_{};
    std::atomic<SYNC_TYPE> m_sync_type_{NONE_SYNC};
    X_DISABLE_COPY_MOVE(XDemuxTask)
};

#endif
