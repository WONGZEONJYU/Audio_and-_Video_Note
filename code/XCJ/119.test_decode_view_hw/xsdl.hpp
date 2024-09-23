//
// Created by Administrator on 2024/8/28.
//

#ifndef INC_106_XVIDEO_VIEW_TEST_XSDL_HPP
#define INC_106_XVIDEO_VIEW_TEST_XSDL_HPP

#include "xvideo_view.hpp"

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

class XSDL : public XVideoView {

    /**
     * 销毁资源
     */
    void DeConstruct();
    bool Start_Rendering();
    bool check_init();

protected:
    /**
     * 初始化渲染窗口 线程安全 可多次调用
     * @param w 窗口宽度
     * @param h 窗口高度
     * @param fmt 绘制的像素格式
     * @param winID 窗口句柄,如果为nullptr,创新新窗口
     * @return true or false
     */
    bool Init(const int &w,const int &h,const Format &fmt) override;

    /**
    * 清理所有申请的资源,包括关闭窗口
    */
    void Close() override;

    /**
     * 渲染图像 线程安全
     * @param datum 渲染的二进制数据
     * @param line_size 一行数据的字节数,对于YUV420P就是Y一行字节数,
     * line_size <=0 就根据宽度和像素格式自动计算出大小,如果有对齐问题,则需手动输入
     * @return ture or false
     */
    bool Draw(const void *datum,int line_size) override;

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
    bool Draw(const uint8_t *y,const int &y_pitch,
              const uint8_t *u,const int &u_pitch,
              const uint8_t *v,const int &v_pitch) override;

    /**
     * 渲染NV12,线程安全
     * @param y
     * @param y_pitch
     * @param uv
     * @param uv_v_pitch
     * @return
     */
    bool Draw(const uint8_t *y,const int &y_pitch,
              const uint8_t *uv,const int &uv_pitch) override;

    /**
     * SDL自己创建的窗口是否退出
     * @return true or false
     */
    [[nodiscard]] bool Is_Exit_Window() const override;

    /**
     * 缩放设置
     * @param w
     * @param h
     */
    void Scale(const int &w,const int &h) override;

protected:
    SDL_Window *m_win{};
    SDL_Renderer *m_renderer{};
    SDL_Texture *m_texture{};

public:
    explicit XSDL() = default;
    ~XSDL() override;
};

#endif
