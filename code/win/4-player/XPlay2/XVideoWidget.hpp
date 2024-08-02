//
// Created by Administrator on 2024/8/1.
//

#ifndef XPLAY2_XVIDEOWIDGET_HPP
#define XPLAY2_XVIDEOWIDGET_HPP

#if defined(__APPLE__) && defined(__MACH__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif

#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>
#include <QMutex>
#include <QSharedPointer>
#include <QVector>
#include <QScopedArrayPointer>
#include <QByteArray>
#include "XHelper.hpp"

class XAVFrame;
using XAVFrame_sptr = std::shared_ptr<XAVFrame>;

class XVideoWidget final : public QOpenGLWidget,
                           protected QOpenGLFunctions
{
//attribute新版已经弃用
//varying新版已经弃用
//一律用in/out代替

//顶点shader
    static inline constexpr auto Vertex_shader{
            R"glsl(
        #version 410 core
        layout(location = 0) in vec2 vertexIn;
        layout(location = 1) in vec2 textureIn;
        out vec2 textureOut;
        void main(){
            gl_Position = vec4(vertexIn,0.0,1.0);
            textureOut = textureIn;
        }
        )glsl"
    };

#if 1
//gl_FragColor新版已经弃用
    static inline constexpr auto Fragment_shader{
            R"glsl(
        #version 410 core
        out vec4 FragColor;
        in vec2 textureOut;
        uniform sampler2D tex_y;
        uniform sampler2D tex_u;
        uniform sampler2D tex_v;
        void main(){
            vec3 yuv;
            vec3 rgb;
            yuv.x = texture(tex_y, textureOut).r;
            yuv.y = texture(tex_u, textureOut).r - 0.5;
            yuv.z = texture(tex_v, textureOut).r - 0.5;
            rgb = mat3(1.0, 1.0, 1.0,
                       0.0, -0.39465, 2.03211,
                       1.13983, -0.58060, 0.0) * yuv;
            FragColor = vec4(rgb, 1.0);
        }
        )glsl"
    };
#else
    static inline constexpr auto tString{
        R"glsl(
        #version 410 core
        out vec4 FragColor;
        in vec2 textureOut;
        uniform sampler2D tex_y;
        uniform sampler2D tex_u;
        uniform sampler2D tex_v;
        void main(){
            float y = texture(tex_y, textureOut).r;
            float u = texture(tex_u, textureOut).r - 0.5;
            float v = texture(tex_v, textureOut).r - 0.5;
            float r = y + 1.13983 * v;
            float g = y - 0.39465 * u - 0.5806 * v;
            float b = y + 2.03211 * u;
            FragColor = vec4(r,g,b,1.0);
        }
        )glsl"
    };
#endif

    //顶点坐标
    static inline constexpr GLfloat ver_tex_coordinate[] {
            //顶点坐标       //纹理(材质)坐标
            -1.0f,-1.0f,    0.0f, 1.0f,
            1.0f,-1.0f,     1.0f, 1.0f,
            -1.0f,1.0f,     0.0f, 0.0f,
            1.0f,1.0f,      1.0f, 0.0f,
    };

    //绘制方向
    static inline constexpr GLuint direction[]{
            0,1,2,
            2,3,1
    };

Q_OBJECT
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int , int ) override;
    void cleanup() noexcept(true);

public:
    explicit XVideoWidget(QWidget*  = nullptr);
    ~XVideoWidget() override;
    /**
     * 该函数在分配纹理对象失败的时候抛异常并会清理所有成员属性
     * @param w
     * @param h
     */
    void Init(const int &w,const int&h) noexcept(false);
    void Repaint(const XAVFrame_sptr &);

private:
    QMutex m_mux;

    //shader程序
    //QOpenGLShaderProgram *m_shader{};
    QSharedPointer<QOpenGLShaderProgram> m_shader;

    //显存空间,用于存储顶点坐标
    //QOpenGLBuffer *m_VBO{},*m_EBO{};
    QSharedPointer<QOpenGLBuffer> m_VBO,m_EBO;

    //顶点内存对象
    //QOpenGLVertexArrayObject *m_VAO{};
    QSharedPointer<QOpenGLVertexArrayObject> m_VAO;

    //材质内存空间,显卡的材质空间
    //QOpenGLTexture *m_textureYUV[3]{};
    QVector<QSharedPointer<QOpenGLTexture>> m_textureYUV;

    //YUV数据空间
    QVector<QByteArray> m_yuv_datum;

    int m_w{},m_h{};
};

#endif
