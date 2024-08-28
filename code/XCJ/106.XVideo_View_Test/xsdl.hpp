//
// Created by Administrator on 2024/8/28.
//

#ifndef INC_106_XVIDEO_VIEW_TEST_XSDL_HPP
#define INC_106_XVIDEO_VIEW_TEST_XSDL_HPP

#include "xvideo_view.hpp"

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

class XSDL : public XVideoView{

public:
    /**
     * 初始化渲染窗口 线程安全
     * @param w 窗口宽度
     * @param h 窗口高度
     * @param fmt 绘制的像素格式
     * @param winID 窗口句柄,如果为nullptr,创新新窗口
     * @return true or false
     */
    bool Init(const int &w,const int &h,const Format &fmt,void *winID) override;
    /**
     * 渲染图像 线程安全
     * @param datum 渲染的二进制数据
     * @param line_size 一行数据的字节数,对于YUV420P就是Y一行字节数,
     * line_size <=0 就根据宽度和像素格式自动计算出大小,如果有对齐问题,则需手动输入
     * @return ture or false
     */
    bool Draw(const void *datum,const int &line_size) override;

protected:
    SDL_Window *m_win{};
    SDL_Renderer *m_renderer{};
    SDL_Texture *m_texture{};
};

#endif
