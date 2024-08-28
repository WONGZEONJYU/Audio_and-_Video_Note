//
// Created by Administrator on 2024/8/28.
//

#ifndef INC_106_XVIDEO_VIEW_TEST_XVIDEO_VIEW_HPP
#define INC_106_XVIDEO_VIEW_TEST_XVIDEO_VIEW_HPP

#include <mutex>

class XVideoView {

public:
    enum Format : int{
        RGBA = 0,
        ARGB,
        YUV420P,
    };
    enum RenderType{
        SDL = 0,
    };

    /**
     * 创建
     * @param renderType
     * @return
     */

    static XVideoView *create(const RenderType &renderType = SDL);

    /**
     * 初始化渲染窗口 线程安全
     * @param w 窗口宽度
     * @param h 窗口高度
     * @param fmt 绘制的像素格式
     * @param winID 窗口句柄,如果为nullptr,创新新窗口
     * @return true or false
     */
    virtual bool Init(const int &w,const int &h,const Format &fmt,void *winID) = 0;

    /**
     * 渲染图像 线程安全
     * @param datum 渲染的二进制数据
     * @param line_size 一行数据的字节数,对于YUV420P就是Y一行字节数,
     * line_size <=0 就根据宽度和像素格式自动计算出大小,如果有对齐问题,则需手动输入
     * @return ture or false
     */
    virtual bool Draw(const void *datum,const int &line_size) = 0;

    virtual ~XVideoView() = default;

protected:
    std::mutex m_mux;
    int m_width{},m_height{};
    Format m_fmt{RGBA};
    void *m_winID{};
};

#endif
