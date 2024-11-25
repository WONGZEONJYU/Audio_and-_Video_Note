#ifndef XVIDEO_VIEW_HPP_
#define XVIDEO_VIEW_HPP_

#include "xhelper.hpp"

class XLIB_API XVideoView {

    void calc_fps();
    static void merge_nv12(uint8_t *,const XAVFrame &);

public:
    //数值与ffmpeg定义一样,详情参考pixfmt.h中的 enum AVPixelFormat
    enum Format : int {
        YUV420P = 0,
        RGB24 = 2,
        BGR24 = 3,
        NV12 = 23,
        NV21 = 24,
        ARGB = 25,
        RGBA = 26,
        ABGR = 27,
        BGRA = 28,
    };

    enum RenderType : int {
        SDL = 0,
    };

    /**
     * 创建XVideoView
     * @param renderType
     * @return
     */
    static XVideoView *create(const RenderType &renderType = SDL);
    static XVideoView_sp create_sp(const RenderType &renderType = SDL);
    /**
     * 初始化渲染窗口 线程安全 可多次调用
     * @param w 窗口宽度
     * @param h 窗口高度
     * @param fmt 绘制的像素格式
     * @return true or false
     */
    virtual bool Init(const int &w,const int &h,
        const Format &fmt = RGBA,const std::string & = "") = 0;

    /**
     * 初始化渲染窗口 线程安全 可多次调用
     * @param parameters
     * @return true or false
     */
    virtual bool Init(const XCodecParameters &parameters,const std::string & = "");

    /**
     * 清理所有申请的资源,包括关闭窗口
     */
    virtual void Close() = 0;

    /**
     * 渲染图像 线程安全
     * @param datum 渲染的二进制数据
     * @param line_size 一行数据的字节数,对于YUV420P就是Y一行字节数,
     * line_size <=0 就根据宽度和像素格式自动计算出大小,如果有对齐问题,则需手动输入
     * @return ture or false
     */
    virtual bool Draw(const void *datum,int line_size = 0) = 0;

    /**
     * 渲染YUV,线程安全
     * @param y
     * @param y_pitch
     * @param u
     * @param u_pitch
     * @param v
     * @param v_pitch
     * @return ture or false
     */
    virtual bool Draw(const uint8_t *y,const int &y_pitch,
                      const uint8_t *u,const int &u_pitch,
                      const uint8_t *v,const int &v_pitch) = 0;

    /**
     * 渲染NV12,线程安全
     * @param y
     * @param y_pitch
     * @param uv
     * @param uv_pitch
     * @return
     */
    virtual bool Draw(const uint8_t *y,const int &y_pitch,
                      const uint8_t *uv,const int &uv_pitch) = 0;

    /**
     * 渲染AVFrame,线程安全,与ffmpeg接口有关
     * @param frame
     * @return
     */
    virtual bool DrawFrame(const XAVFrame &frame);

    /**
     * 缩放设置
     * @param w
     * @param h
     */
    virtual void Scale(const int &w,const int &h){
        m_scale_w_ = w;
        m_scale_h_ = h;
    }

    /**
     * 设置Render位置
     * @param x
     * @param y
     */
    virtual void SetPos(const int &x,const int &y){
        m_x_ = x;
        m_y_ = y;
    }

    /**
     * 窗口退出
     * @return ture or false
     */
    [[nodiscard]] virtual bool Is_Exit_Window() const = 0;

    /**
     * 返回当前播放帧率
     * @return 当前播放帧率
     */
    [[nodiscard]] virtual int Render_Fps() const {
        return m_render_fps_;
    }

    /**
     * 打开文件
     * @param file_path
     * @return ture or false
     */
    virtual bool Open(const std::string &file_path);

    /**
     * 读取一帧数据,并维护AVFrame空间,
     * 每次调用会覆盖上一次数据
     * @return XAVFrame_sptr
     */
    [[nodiscard]] virtual XAVFrame_sp Read();

    /**
     * 设置窗口句柄
     * @param win_id
     */
    virtual void Set_Win_ID(void *win_id){
        m_winID_ = win_id;
    }

    virtual void ShowWindow(){}
protected:
    std::mutex m_mux_;
    std::atomic_int64_t m_begin_time_{};
    std::atomic_int m_width_{},m_height_{},
                m_x_{},m_y_{},
                m_scale_w_{},m_scale_h_{},
                m_count_{},m_render_fps_{},
                m_pixel_Byte_size_{4};

    std::atomic<Format> m_fmt_{RGBA};
    std::atomic<void*> m_winID_{};
private:
    std::ifstream m_ifs_;
    XAVFrame_sp m_frame_;
    std::vector<uint8_t> m_cache_;

protected:
    explicit XVideoView() = default;
public:
    static int64_t Get_time_ms();
    static void MSleep(const uint64_t &);
    X_DISABLE_COPY_MOVE(XVideoView);
    virtual ~XVideoView() = default;
};

#endif
