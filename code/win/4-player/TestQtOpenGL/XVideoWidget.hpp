//
// Created by wong on 2024/7/21.
//

#ifndef XPLAY2_XVIDEOWIDGET_HPP
#define XPLAY2_XVIDEOWIDGET_HPP

#if defined(__APPLE__) && defined(__MACH__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif

#include <QtOpenGLWidgets/QtOpenGLWidgets>
#include <QOpenGLShaderProgram>
#include <QFile>
//#include <QOpenGLWidget>

#if defined(__APPLE__) && defined(__MACH__)
#include <QOpenGLFunctions_4_0_Core>
#else
#include <QOpenGLFunctions>
#endif

#if defined(__APPLE__) && defined(__MACH__)
class XVideoWidget : public QOpenGLWidget,
        protected QOpenGLFunctions_4_0_Core
#else
class XVideoWidget : public QOpenGLWidget,
        protected QOpenGLFunctions
#endif
{
Q_OBJECT
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int , int ) override;
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
};

#endif