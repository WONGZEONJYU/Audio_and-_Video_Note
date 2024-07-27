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
#include <QOpenGLExtraFunctions>

class XVideoWidget final : public QOpenGLWidget,
        protected QOpenGLExtraFunctions
{
    //顶点shader
    static inline constexpr auto vString{
            R"glsl(
    #version 410
    layout(location = 0)in vec2 vertexIn;
    layout(location = 1)in vec2 textureIn;
    out vec2 textureOut;
    void main(void){
        gl_Position = vec4(vertexIn,0.0,1.0);
        textureOut = textureIn;
    }
)glsl"
};

//gl_FragColor
//片元shader
    static inline constexpr auto tString{
            R"glsl(
    #version 410
    in vec2 textureOut;
    uniform sampler2D tex_y;
    uniform sampler2D tex_u;
    uniform sampler2D tex_v;
    out vec4 FragColor;
    void main(void){
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

    //顶点坐标
    static inline constexpr GLfloat ver[] {
            //顶点坐标                  //纹理坐标
            -1.0f,-1.0f,  0.0f, 1.0f,
            1.0f,-1.0f, 1.0f, 1.0f,
            -1.0f,1.0f, 0.0f, 0.0f,
            1.0f,1.0f, 1.0f, 0.0f,
    };

    static inline constexpr GLfloat tex[] {

            0.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,
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
    uint32_t m_vao{},m_vbo{},m_ebo{};

    //shader中yuv变量地址
    GLint m_unis[3]{};
    //opengl的texture地址
    GLuint m_texs[3]{};

    //材质内存空间
    //uint8_t *m_datas[3]{};

    int m_w{240},m_h{128};

    QFile m_file;

    QTimer timer;
};

#endif
