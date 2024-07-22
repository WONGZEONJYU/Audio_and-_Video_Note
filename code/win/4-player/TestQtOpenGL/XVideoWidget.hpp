//
// Created by wong on 2024/7/21.
//

#ifndef XPLAY2_XVIDEOWIDGET_HPP
#define XPLAY2_XVIDEOWIDGET_HPP

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <QtOpenGLWidgets/QtOpenGLWidgets>
#include <QOpenGLShaderProgram>
//#include <QOpenGLWidget>
//#include <QOpenGLFunctions>

class XVideoWidget : public QOpenGLWidget,
        protected QOpenGLFunctions
{
Q_OBJECT
    void initializeGL() override;
    void resizeGL(int , int ) override;
    void paintGL() override;
public:
    explicit XVideoWidget(QWidget*  = nullptr);
    ~XVideoWidget() override;

private:
    //shader程序
    QOpenGLShaderProgram m_program;
    //shader中yuv变量地址
    GLuint m_unis[3]{};
    //opengl的texture地址
    GLuint m_texs[3]{};

    //材质内存空间
    uint8_t *m_datas[3]{};

    int m_w{240},m_h{180};
};

#endif
