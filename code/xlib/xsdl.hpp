#ifndef XSDL_HPP_
#define XSDL_HPP_

#include "xvideo_view.hpp"

class XSDL : public XVideoView {

    friend class XVideoView;
    /**
     * 销毁资源
     */
    void DeConstruct();
    bool Start_Rendering() const;
    bool check_init() const ;

protected:
    /**
     * 初始化渲染窗口 线程安全 可多次调用
     * @param w 窗口宽度
     * @param h 窗口高度
     * @param fmt 绘制的像素格式
     * @param win_title 窗口标题,可以不填
     * @return true or false
     */
    bool Init(const int &w,const int &h,const Format &fmt,const std::string &win_title) override;

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
    bool Draw(const void *datum,int line_size) override ;

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
     * @param uv_pitch
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

    void ShowWindow() override;

protected:
    SDL_Window *m_win_{};
    SDL_Renderer *m_renderer_{};
    SDL_Texture *m_texture_{};
    explicit XSDL() = default;
public:
    ~XSDL() override;
    X_DISABLE_COPY_MOVE(XSDL)
};

#endif
