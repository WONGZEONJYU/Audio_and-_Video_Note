#ifndef XPLAY_HPP
#define XPLAY_HPP

#include "xtools.hpp"
#include "xdemuxtask.hpp"
#include "xdecodetask.hpp"

class XLIB_API XPlay : public XThread {
     X_DISABLE_COPY_MOVE(XPlay)
     void Main() override;
     void Do(XAVPacket &) override;
public:
     /**
      * 打开音视频 初始化和渲染
      * @param url
      * @param win_id
      * @return ture or false
      */
     bool Open(const std::string &url,void *win_id = {});

     /**
      * 启动线程
      */
     void Start() override;

     void Update();

     explicit XPlay() = default;
     ~XPlay() override ;
protected:
     XDemuxTask m_demuxTask_;
     XDecodeTask m_video_decode_task_,
          m_audio_decode_task_;
     XVideoView_sp m_videoView_{};
};

#endif
