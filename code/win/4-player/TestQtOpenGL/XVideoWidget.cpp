//
// Created by wong on 2024/7/21.
//

#include "XVideoWidget.hpp"
//自动加双引号
#define GET_STR(args) #args
static inline constexpr auto A_VER{3};
static inline constexpr auto T_VER{4};

//顶点shader
static inline constexpr auto vString{
GET_STR(
    attribute vec4 vertexIn; //顶点坐标,attribute可以用in代替
    attribute vec2 textureIn; //材质坐标
    varying vec2 textureOut; //片元和顶点共享坐标
    void main(void){
        gl_Position = vertexIn;
        textureOut = textureIn;
    }
)};

//片元shader
static inline constexpr auto tString{
GET_STR(
    varying vec2 textureOut;
    uniform sampler2D tex_y;
    uniform sampler2D tex_u;
    uniform sampler2D tex_v;
    void main(void){
        vec3 yuv;
        vec3 rgb;
        yuv.x = texture2D(tex_y,textureOut).r;
        yuv.y = texture2D(tex_u,textureOut).r - 0.5;
        yuv.z = texture2D(tex_v,textureOut).r - 0.5;
        rgb = mat3(1.0,1.0,1.0,0.0,
                   -0.39465,2.03211,1.13983,-0.58060,0.0) * yuv;
        gl_FragColor = vec4(rgb,1.0);
    }
)};

XVideoWidget::XVideoWidget(QWidget *parent):
    QOpenGLWidget(parent){
}

XVideoWidget::~XVideoWidget() {
    for (auto & i:m_datas) {
        delete [] i;
        i = nullptr;
    }
    //std::fill_n(m_datas,std::size(m_datas), nullptr);
}

//初始化opengl
void XVideoWidget::initializeGL() {

    qDebug() << __FUNCTION__;
    //初始化opengl
    initializeOpenGLFunctions();

    //program加载shader(顶点和片元)脚本
    //片元(像素)
    qDebug() << m_program.addShaderFromSourceCode(QOpenGLShader::Fragment,tString);
    //顶点shader
    qDebug() << m_program.addShaderFromSourceCode(QOpenGLShader::Vertex,vString);

    //设置顶点坐标的变量
    m_program.bindAttributeLocation(GET_STR(vertexIn),A_VER);

    //设置材质坐标
    m_program.bindAttributeLocation(GET_STR(textureIn),T_VER);

    //编译shader
    qDebug() << "program.link() = " << m_program.link();
    //绑定
    qDebug() << "program.bind() = " << m_program.bind();

    //传递顶点和材质坐标
    //顶点坐标
    static constexpr GLfloat ver[]{
//逆时针,
        -1.0f,-1.0,
        1.0f,-1.0f,
        -1.0f,1.0f,
        1.0f,1.0f
//顺时针
//        1.0f,-1.0f,
//        -1.0f,-1.0,
//        1.0f,1.0f,
//        -1.0f,1.0f
    };

    static constexpr GLfloat tex[]{
//逆时针
            0.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,
//顺时针
//            1.0f,0.0f,
//            0.0f,0.0f,
//            1.0f,1.0f,
//            0.0f,1.0f,
    };

    //顶点
    glVertexAttribPointer(A_VER, 2, GL_FLOAT, 0, 0, ver);
    glEnableVertexAttribArray(A_VER);

    //材质
    glVertexAttribPointer(T_VER, 2, GL_FLOAT, 0, 0, tex);
    glEnableVertexAttribArray(T_VER);

    //从shader获取材质
    m_unis[0] = m_program.uniformLocation(GET_STR(tex_y));
    m_unis[1] = m_program.uniformLocation(GET_STR(tex_u));
    m_unis[2] = m_program.uniformLocation(GET_STR(tex_v));

    //创建材质(YUV的材质)
    glGenTextures(static_cast<GLsizei>(std::size(m_texs)),m_texs);

    /*************************************Y********************************************/
    glBindTexture(GL_TEXTURE_2D,m_texs[0]);
    //放大过滤,线性插值
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    //创建材质显卡空间
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,m_w,m_h,
                 0,GL_RED,GL_UNSIGNED_BYTE, nullptr);
    /*************************************Y********************************************/

    /*******************************************U***************************************/
    glBindTexture(GL_TEXTURE_2D,m_texs[1]);
    //放大过滤,线性插值
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    //创建材质显卡空间
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,m_w/2,m_h/2,
                 0,GL_RED,GL_UNSIGNED_BYTE, nullptr);
    /*******************************************U***************************************/

    /*****************************************V******************************************/
    glBindTexture(GL_TEXTURE_2D,m_texs[2]);
    //放大过滤,线性插值
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    //创建材质显卡空间
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,m_w/2,m_h/2,
                 0,GL_RED,GL_UNSIGNED_BYTE, nullptr);
    /*****************************************V******************************************/

    //分配材质内存空间
    m_datas[0] = new uint8_t[m_w * m_h]{}; //Y
    m_datas[1] = new uint8_t[m_w * m_h / 4]{}; //U
    m_datas[2] = new uint8_t[m_w * m_h / 4]{}; //V

    //QOpenGLWidget::initializeGL();
}

void XVideoWidget::resizeGL(int w, int h) {
    qDebug() << __FUNCTION__ << " w:" << w << " h:" << h;
    //QOpenGLWidget::resizeGL(w, h);
}

void XVideoWidget::paintGL() {
    qDebug() << __FUNCTION__;
    //QOpenGLWidget::paintGL();

}
