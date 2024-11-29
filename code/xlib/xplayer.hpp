#ifndef XPLAY_HPP
#define XPLAY_HPP

#include "xtools.hpp"
#include "xdemuxtask.hpp"
#include "xdecodetask.hpp"
#include <functional>

class XLIB_API XPlayer : public XThread {
     X_DISABLE_COPY_MOVE(XPlayer)
    static inline constexpr auto TOLERANCE{0.05};
     void Main() override;
     void Do(XAVPacket &) override;
public:
     /**
      * 打开音视频 初始化和渲染
      * 如果设置为外部显,则需调用Update的重载版本
      * @param url
      * @param win_id
      * @param ex_ 如设置为true(外部显示),win_id参数视为无效
      * @return ture or false
      */
    bool Open(const std::string &url,void *win_id = {},const bool &ex_ = false);

     /**
      * 停止线程
      */
     void Stop() override;
     /**
      * 启动线程
      */
    void Start() override;

     /**
      * 更新显示和更新音频输出数据
      */
    void Update();

     /**
      * 用于支持外部的视频显示接口
      * @param f 视频帧
      */
    void Update(auto &&f) {
        Update();
         if (const auto vf{m_video_decode_task_.CopyFrame()}) {
             f(*vf);
         }
     }

     /**
      * 检查SDL窗口是否退出,必须在主线程调用
      * @return ture or false
      */
    virtual bool win_is_exit();

     /**
      * 外部显示的时候,用于获取到视频参数
      * @return XCodecParameters_sp or nullptr
      */
    XCodecParameters_sp get_video_params() const;

    void SetSpeed(const float &speed);

    auto total_ms() const{return m_total_ms_.load();}

    auto pos_ms() const{return m_pos_ms_.load();}

    explicit XPlayer() = default;
    ~XPlayer() override ;

protected:
    XDemuxTask m_demuxTask_;
    XDecodeTask m_video_decode_task_,
        m_audio_decode_task_;
    XVideoView_sp m_videoView_{};
    std::atomic_bool m_is_open_{};
    XCodecParameters_sp m_video_params_{};
    std::atomic_int_fast64_t m_total_ms_{},m_pos_ms_{};
};

#endif
