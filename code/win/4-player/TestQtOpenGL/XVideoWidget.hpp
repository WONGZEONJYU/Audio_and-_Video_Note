//
// Created by wong on 2024/7/21.
//

#ifndef XPLAY2_XVIDEOWIDGET_HPP
#define XPLAY2_XVIDEOWIDGET_HPP

#include <QtOpenGLWidgets/QtOpenGLWidgets>
#include <QOpenGLShaderProgram>

class XVideoWidget : public QOpenGLWidget,
        protected QOpenGLFunctions
{
Q_OBJECT
    void initializeGL() override;
    void resizeGL(int , int ) override;
    void paintGL() override;
public:
    explicit XVideoWidget(QWidget*  = nullptr);
    ~XVideoWidget() override = default;

private:
    //shader程序
    QOpenGLShaderProgram program;
};

#endif
