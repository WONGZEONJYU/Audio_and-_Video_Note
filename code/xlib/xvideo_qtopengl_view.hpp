#ifndef XVIDEO_WIDGET_HPP
#define XVIDEO_WIDGET_HPP

#if 0
#ifdef MACOS

#if defined(__APPLE__) && defined(__MACH__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMutex>
#include <QSharedPointer>
#include <QVector>
#include "xhelper.hpp"

class QOpenGLShaderProgram;
class QOpenGLBuffer;
class QOpenGLVertexArrayObject;
class QOpenGLTexture;

class XVideoWidget : public QOpenGLWidget,
                           protected QOpenGLFunctions
{
Q_OBJECT
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int , int ) override;
    void cleanup() noexcept(true);
    void copy_y(const XAVFrame &);
    void copy_uv(const XAVFrame &);

public:
    explicit XVideoWidget(QWidget* = {});
    ~XVideoWidget() override;
    /**
     * 该函数在分配纹理对象失败的时候抛异常并会清理所有成员属性
     * @param w
     * @param h
     */
    bool Init(const int &w,const int&h) noexcept(true);

    /**
     * 通过视频参数去初始化
     * @param parameters
     * @return true or false
     */
    bool Init(const XCodecParameters &parameters) noexcept(true);
    /**
     * 绘制YUV数据
     */
    void Repaint(const XAVFrame &) ;

private:
    QMutex m_mux_;

    //shader程序
    QSharedPointer<QOpenGLShaderProgram> m_shader_;

    //显存空间,用于存储顶点坐标
    QSharedPointer<QOpenGLBuffer> m_VBO_,m_EBO_;

    //顶点内存对象
    QSharedPointer<QOpenGLVertexArrayObject> m_VAO_;

    //材质内存空间,显卡的材质空间
    QVector<QSharedPointer<QOpenGLTexture>> m_textureYUV_;

    //YUV数据空间
    QVector<QByteArray> m_yuv_datum_;

    QAtomicInt m_w_{},m_h_{},
                    m_half_w_{},m_half_h_{};
};

#endif

#endif
#endif