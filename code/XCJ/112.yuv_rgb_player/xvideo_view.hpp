//
// Created by Administrator on 2024/8/28.
//

#ifndef INC_106_XVIDEO_VIEW_TEST_XVIDEO_VIEW_HPP
#define INC_106_XVIDEO_VIEW_TEST_XVIDEO_VIEW_HPP

#include <mutex>
#include <atomic>
#include <memory>
#include <fstream>
#include "XHelper.hpp"

class XAVFrame;
using XAVFrame_sptr = typename std::shared_ptr<XAVFrame>;

class XVideoView {

    void calc_fps();

public:
    enum Format : int{
        RGBA = 0,
        ARGB,
        YUV420P,
        BGRA,
    };
    enum RenderType{
        SDL = 0,
    };

    /**
     * 创建XVideoView
     * @param renderType
     * @return
     */
    static XVideoView *create(const RenderType &renderType = SDL);

    /**
     * 初始化渲染窗口 线程安全 可多次调用
     * @param w 窗口宽度
     * @param h 窗口高度
     * @param fmt 绘制的像素格式
     * @return true or false
     */
    virtual bool Init(const int &w,const int &h,const Format &fmt = RGBA) = 0;

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
    virtual bool Draw(const uint8_t *y,int y_pitch,
                      const uint8_t *u,int u_pitch,
                      const uint8_t *v,int v_pitch) = 0;

    /**
     * 渲染AVFrame,线程安全
     * @param frame
     * @return
     */
    virtual bool DrawFrame(const XAVFrame_sptr &frame);

    /**
     * 缩放设置
     * @param w
     * @param h
     */
    virtual void Scale(const int &w,const int &h){
        m_scale_w = w;
        m_scale_h = h;
    }

    [[nodiscard]] virtual bool Is_Exit_Window() const = 0;

    [[nodiscard]] virtual int view_fps() const {
        return m_view_fps;
    };

    /**
     * 打开文件
     * @param file_path
     * @return ture or false
     */
    bool Open(const std::string &file_path);

    /**
     * 设置窗口句柄
     * @param win_id
     */
    void Set_Win_ID(void *win_id){
        m_winID = win_id;
    }

protected:
    std::mutex m_mux;
    std::atomic_int64_t m_begin_time{};
    std::atomic_int m_width{},m_height{},
                m_scale_w{},m_scale_h{},
                m_count{},m_view_fps{};

    std::atomic<Format> m_fmt{RGBA};
    std::atomic<void*> m_winID{};

private:
    std::ifstream m_ifs;


public:
    static int64_t Get_time_ms();
    static void MSleep(const uint64_t &);
    explicit XVideoView() = default;
    virtual ~XVideoView() = default;
    X_DISABLE_COPY_MOVE(XVideoView);
};

#endif
