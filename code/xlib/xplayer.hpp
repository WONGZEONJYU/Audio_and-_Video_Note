#ifndef XPLAY_HPP
#define XPLAY_HPP

#include "xtools.hpp"
#include "xdemuxtask.hpp"
#include "xdecodetask.hpp"
#include <functional>

class XLIB_API XPlayer : public XThread {
     X_DISABLE_COPY_MOVE(XPlayer)
    void Main() override;
    void Do(XAVPacket &) override;
    bool open_helper(const std::string &url);
public:

     /**
      * 打开音视频 初始化和渲染
      * 使用本库显示
      * @param url
      * @param win_id
      * @return ture or false
      */
    bool Open(const std::string &url,
              void *win_id);

    /**
     * 打开音视频 初始化和渲染
     * 使用外部显示,显示与本库无关
     * @param url
     * @param ex_func
     * @return ture or false
     */
    using callable = std::function<void(const XAVFrame &)>;
    bool Open(const std::string &url,callable &&ex_func){
        if (!ex_func){
            return {};
        }
        m_ex_func_ = std::move(ex_func);
        return Open(url,{});
    }
     /**
      * 停止线程
      */
     void Stop() override;
     /**
      * 启动线程
      */
    void Start() override;

    /**
     * 暂停
     * @param b
     */
    void pause(const bool &b) override;

    [[maybe_unused]] bool Seek(const int64_t &ms);

     /**
      * 更新显示和更新音频输出数据
      */
    [[maybe_unused]] void Update();

     /**
      * 检查SDL窗口是否退出,必须在主线程调用
      * @return ture or false
      */
    [[maybe_unused]] virtual bool win_is_exit();

     /**
      * 外部显示的时候,用于获取到视频参数
      * 需先Open才可能获取到参数
      * @return XCodecParameters_sp or nullptr
      */
    [[maybe_unused]] XCodecParameters_sp get_video_params() const;

    [[maybe_unused]] void SetSpeed(const float &speed);

    [[maybe_unused]] auto total_ms() const{return m_total_ms_.load();}

    [[maybe_unused]] auto pos_ms() const{return m_pos_ms_.load();}

    explicit XPlayer() = default;
    ~XPlayer() override ;

protected:
    std::mutex m_mux_;
    XDemuxTask m_demuxTask_;
    XDecodeTask m_video_decode_task_,
        m_audio_decode_task_;
    XVideoView_sp m_videoView_{};
    std::atomic_bool m_is_open_{};
    XCodecParameters_sp m_video_params_{};
    std::atomic_int_fast64_t m_total_ms_{},m_pos_ms_{};
    callable m_ex_func_{};
};

#endif
