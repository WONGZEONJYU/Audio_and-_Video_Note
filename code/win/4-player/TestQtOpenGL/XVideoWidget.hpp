//
// Created by wong on 2024/7/21.
//

#ifndef XPLAY2_XVIDEOWIDGET_HPP
#define XPLAY2_XVIDEOWIDGET_HPP

#if defined(__APPLE__) && defined(__MACH__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif

#include <QOpenGLShaderProgram>
#include <QFile>
#include <QOpenGLWidget>
#include <QTimer>
#include <QOpenGLFunctions>

class XVideoWidget final : public QOpenGLWidget,protected QOpenGLFunctions
{
    //顶点坐标
    static inline constexpr int ver[] {
//逆时针,
            -1,-1,
            1,-1,
            -1,1,
            1,1,
//顺时针
//        1.0f,-1.0f,0.0f,
//        -1.0f,-1.0,0.0f,
//        1.0f,1.0f,0.0f,
//        -1.0f,1.0f,0.0f
    };

    static inline constexpr int tex[] {
//逆时针
            0, 1,
            1, 1,
            0, 0,
            1, 0,
//顺时针
//            1.0f,0.0f,
//            0.0f,0.0f,
//            1.0f,1.0f,
//            0.0f,1.0f
    };

Q_OBJECT
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int , int ) override;
    void checkOpenGLError(const char* , const char* ,const int &);
public:
    explicit XVideoWidget(QWidget*  = nullptr);
    ~XVideoWidget() override;

private:
    //shader程序
    QOpenGLShaderProgram m_program;

    //shader中yuv变量地址
    GLint m_unis[3]{};
    //opengl的texture地址
    GLuint m_texs[3]{};

    //材质内存空间
    uint8_t *m_datas[3]{};

    int m_w{240},m_h{128};

    QFile m_file;

    QTimer timer;
};

#endif
