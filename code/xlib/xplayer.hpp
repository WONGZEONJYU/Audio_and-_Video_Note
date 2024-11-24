#ifndef XPLAY_HPP
#define XPLAY_HPP

#include "xtools.hpp"
#include "xdemuxtask.hpp"
#include "xdecodetask.hpp"

class XLIB_API XPlayer : public XThread {
     X_DISABLE_COPY_MOVE(XPlayer)
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

     /**
      * 更新显示和更新音频输出数据
      */
     void Update();

     /**
      * 检查SDL窗口是否退出
      * @return ture or false
      */
     bool win_is_exit();

     explicit XPlayer() = default;
     ~XPlayer() override ;
protected:
    XDemuxTask m_demuxTask_;
    XDecodeTask m_video_decode_task_,
            m_audio_decode_task_;
    XVideoView_sp m_videoView_{};
    std::atomic_bool m_is_open_{};
};

#endif
