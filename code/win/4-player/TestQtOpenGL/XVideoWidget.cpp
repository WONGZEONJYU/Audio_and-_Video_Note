//
// Created by wong on 2024/7/21.
//

#include "XVideoWidget.hpp"

#define GL_CHECK(stmt) do { \
        stmt; \
        checkOpenGLError(#stmt, __FILE__, __LINE__); \
    } while (0)

//自动加双引号
#define GET_STR(args) #args
static inline constexpr int A_VER{0};
static inline constexpr int T_VER{1};

//attribute
//varying



XVideoWidget::XVideoWidget(QWidget *parent):QOpenGLWidget(parent){

}

XVideoWidget::~XVideoWidget() {

}

//初始化opengl
void XVideoWidget::initializeGL() {
    qDebug() << "begin " << __FUNCTION__ ;
    //初始化opengl
    initializeOpenGLFunctions();

    qDebug() << "OpenGL version: " << reinterpret_cast<const char*>(glGetString(GL_VERSION));
    qDebug() << "OpenGL GLSL version: " <<  reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));

    //program加载shader(顶点和片元)脚本

    //片元(像素)
    qDebug() << m_program.addShaderFromSourceCode(QOpenGLShader::Fragment,tString);
    qDebug() << m_program.log();

    //顶点shader
    qDebug() << m_program.addShaderFromSourceCode(QOpenGLShader::Vertex,vString);
    qDebug() << m_program.log();

    //编译shader
    qDebug() << "program.link() = " << m_program.link();
    qDebug() << m_program.log();
    //绑定
    qDebug() << "program.bind() = " << m_program.bind();
    qDebug() << m_program.log();

    GL_CHECK(glUseProgram(m_program.programId()));

    glGenBuffers(1,&m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER,m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ver), ver, GL_STATIC_DRAW);

    glGenVertexArrays(1,&m_vao);
    glBindVertexArray(m_vao);

    //传递顶点和材质坐标
    const auto vertexIn_num {m_program.attributeLocation(GET_STR(vertexIn))};
    qDebug() << "vertexIn_num = " << vertexIn_num;
    //顶点
    GL_CHECK(glVertexAttribPointer(vertexIn_num, 2, GL_FLOAT, GL_FALSE,
                                   4 * sizeof(float ), nullptr));
    GL_CHECK(glEnableVertexAttribArray(vertexIn_num));

    const auto textureIn_num {m_program.attributeLocation(GET_STR(textureIn))};
    qDebug() << "textureIn_num = " << textureIn_num;
    //材质
    GL_CHECK(glVertexAttribPointer(textureIn_num, 2, GL_FLOAT, GL_FALSE,
                                   4 * sizeof(float ), reinterpret_cast<void *>(2 * sizeof (float ))));
    GL_CHECK(glEnableVertexAttribArray(textureIn_num));

    //从shader获取材质
    m_unis[0] = m_program.uniformLocation(GET_STR(tex_y));
    m_unis[1] = m_program.uniformLocation(GET_STR(tex_u));
    m_unis[2] = m_program.uniformLocation(GET_STR(tex_v));

    //创建材质(YUV的材质)
    GL_CHECK(glGenTextures(static_cast<GLsizei>(std::size(m_texs)),m_texs));

    /*************************************Y********************************************/
    GL_CHECK(glBindTexture(GL_TEXTURE_2D,m_texs[0]));
    //放大过滤,线性插值,GL_NEAREST(效率高,但马赛克严重)
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));
    //创建材质显卡空间
    GL_CHECK(glTexImage2D(GL_TEXTURE_2D,0,GL_RED,m_w,m_h,
                 0,GL_RED,GL_UNSIGNED_BYTE, nullptr));
    /*************************************Y********************************************/

    /*******************************************U***************************************/
    GL_CHECK(glBindTexture(GL_TEXTURE_2D,m_texs[1]));
    //放大过滤,线性插值
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));
    //创建材质显卡空间
    GL_CHECK(glTexImage2D(GL_TEXTURE_2D,0,GL_RED,m_w / 2,m_h / 2,
                 0,GL_RED,GL_UNSIGNED_BYTE, nullptr));
    /*******************************************U***************************************/

    /*****************************************V******************************************/
    GL_CHECK(glBindTexture(GL_TEXTURE_2D,m_texs[2]));
    //放大过滤,线性插值
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));
    //创建材质显卡空间
    GL_CHECK(glTexImage2D(GL_TEXTURE_2D,0,GL_RED,m_w / 2,m_h / 2,
                 0,GL_RED,GL_UNSIGNED_BYTE, nullptr));
    /*****************************************V******************************************/

    GL_CHECK(glUniform1i(m_unis[0], 0));
    GL_CHECK(glUniform1i(m_unis[1], 1));
    GL_CHECK(glUniform1i(m_unis[2], 2));

    glBindVertexArray(0);

    m_file.setFileName(GET_STR(out240x128.yuv));
    if (!m_file.open(QFile::ReadOnly)){
        throw std::runtime_error(GET_STR(out240x128.yuv file open failed!));
    }

    void (XVideoWidget::*f)(){&XVideoWidget::update};
    connect(&timer,&QTimer::timeout,this,f);

    timer.start(40);

    qDebug() << "end " << __FUNCTION__ ;
}

void XVideoWidget::paintGL() {

    qDebug() << "begin: " << __FUNCTION__;

    glBindVertexArray(m_vao);

    if (m_file.atEnd()){
        m_file.seek(0);
    }
    auto data{m_file.read(m_w*m_h)};
    /****************************************y****************************************/
    GL_CHECK(glActiveTexture(GL_TEXTURE0));//激活了0层材质
    GL_CHECK(glBindTexture(GL_TEXTURE_2D,m_texs[0])); //0层绑定到Y材质
    //修改材质内容(复印内存中内容)
    GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D,0,0,0,m_w,m_h,
                    GL_RED,GL_UNSIGNED_BYTE,data));
    //与shader uni变量关联
    //GL_CHECK(glUniform1i(m_unis[0], 0));
    /****************************************y****************************************/
    data = std::move(m_file.read(m_w*m_h/4));
    /****************************************u****************************************/
    GL_CHECK(glActiveTexture(GL_TEXTURE0 + 1));//激活了1层材质
    GL_CHECK(glBindTexture(GL_TEXTURE_2D,m_texs[1])); //1层绑定到U材质
    //修改材质内容(复印内存中内容)
    GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D,0,0,0,m_w / 2,m_h / 2,
                    GL_RED,GL_UNSIGNED_BYTE,data));
    //与shader uni变量关联
    //GL_CHECK(glUniform1i(m_unis[1], 1));
    /****************************************u****************************************/

    data = std::move(m_file.read(m_w*m_h/4));
    /****************************************v****************************************/
    GL_CHECK(glActiveTexture(GL_TEXTURE0 + 2));//激活了2层材质
    GL_CHECK(glBindTexture(GL_TEXTURE_2D,m_texs[2])); //2层绑定到V材质
    //修改材质内容(复印内存中内容)
    GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D,0,0,0,m_w / 2,m_h / 2,
                    GL_RED,GL_UNSIGNED_BYTE,data));
    //与shader uni变量关联
    //GL_CHECK(glUniform1i(m_unis[2], 2));
    /****************************************v****************************************/

    GL_CHECK(glDrawArrays(GL_TRIANGLE_STRIP,0,4));

    glBindVertexArray(0);

    qDebug() << "end: " << __FUNCTION__;
}

void XVideoWidget::resizeGL(int w, int h) {
    //qDebug() << __FUNCTION__ << " w:" << w << " h:" << h;
    glViewport(0, 0, w, h);
}

void XVideoWidget::checkOpenGLError(const char* stmt, const char* fname,const int &line) {
    const auto err{this->glGetError()};
    if(GL_NO_ERROR != err) {
        qDebug() << "OpenGL error " << err << " at " << fname << ":" << line << " - for " << stmt;
    }
}
