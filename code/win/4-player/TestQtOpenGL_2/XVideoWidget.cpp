//
// Created by wong on 2024/7/21.
//

#include "XVideoWidget.hpp"
#include <QByteArray>
#include <QCoreApplication>

#define GL_CHECK(stmt) do { \
        stmt; \
        checkOpenGLError(#stmt, __FILE__, __LINE__); \
    } while (0)

//自动加双引号
#define GET_STR(args) #args

XVideoWidget::XVideoWidget(QWidget *parent):QOpenGLWidget(parent){

}

XVideoWidget::~XVideoWidget() {
    qDebug() << __FUNCTION__ ;
    cleanup();
}

//初始化opengl
void XVideoWidget::initializeGL() {

    qDebug() << "begin " << __FUNCTION__ ;

    m_file.setFileName(GET_STR(out240x128.yuv));

    try {
        if (!m_file.open(QFile::ReadOnly)){
            throw std::runtime_error(GET_STR(out240x128.yuv file open failed!));
        }
    } catch (const std::exception &e) {
        qDebug() << e.what();
        return;
    }

    //初始化opengl
    initializeOpenGLFunctions();

    qDebug() << "OpenGL version: " << reinterpret_cast<const char*>(glGetString(GL_VERSION));
    qDebug() << "OpenGL GLSL version: " <<  reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));

    //program加载shader(顶点和片元)脚本
    m_shader.reset(new QOpenGLShaderProgram());
    //m_shader->create(); //通常不需要手动调用，因为 addShader, link, 等函数会隐式创建。
    //顶点shader
    qDebug() << m_shader->addShaderFromSourceCode(QOpenGLShader::Vertex,Vertex_shader);
    qDebug() << m_shader->log();

    //片元shader
    qDebug() << m_shader->addShaderFromSourceCode(QOpenGLShader::Fragment,Fragment_shader);
    qDebug() << m_shader->log();

    //编译shader
    qDebug() << "m_shader->link() = " << m_shader->link();
    qDebug() << m_shader->log();
    //绑定shader
    qDebug() << "program.bind() = " << m_shader->bind();
    qDebug() << m_shader->log();

    //VAO VBO EBO详情看链接到解释
    //https://blog.csdn.net/weixin_44179561/article/details/124275761
    //https://learnopengl-cn.readthedocs.io/zh/latest/01%20Getting%20started/05%20Shaders/
    m_VAO.reset(new QOpenGLVertexArrayObject());
//    m_VAO->create();
//    m_VAO->bind();
    QOpenGLVertexArrayObject::Binder vao(m_VAO.get());//代替m_VAO->create(); m_VAO->bind();m_VAO->release();
    //采用RAII技术

    m_VBO.reset(new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer));
    m_VBO->create();
    qDebug() << "m_VBO->bind() = " << m_VBO->bind();
    m_VBO->allocate(ver, sizeof(ver));
    m_VBO->setUsagePattern(QOpenGLBuffer::StaticDraw);

    m_EBO.reset(new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer));
    m_EBO->create();
    qDebug() << "m_EBO->bind() = " << m_EBO->bind();
    m_EBO->allocate(indexs, sizeof(indexs));

    //传递顶点和材质坐标
    const auto vertexIn_num {m_shader->attributeLocation(GET_STR(vertexIn))};
    qDebug() << "vertexIn_num = " << vertexIn_num;
    //顶点坐标设置
    m_shader->setAttributeArray(vertexIn_num, //顶点着色器vertexIn的location编号
                                GL_FLOAT, //坐标类型
                                nullptr, //坐标数组的起始位置
                                2, //坐标数量,例如(x,y)
                                4* sizeof(float )); //访问偏移
    m_shader->enableAttributeArray(vertexIn_num); //启用顶点坐标

//    GL_CHECK(glVertexAttribPointer(vertexIn_num, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float ), nullptr));
//    GL_CHECK(glEnableVertexAttribArray(vertexIn_num));

    const auto textureIn_num {m_shader->attributeLocation(GET_STR(textureIn))};
    qDebug() << "textureIn_num = " << textureIn_num;

    //纹理(材质)坐标设置
    m_shader->setAttributeArray(textureIn_num, //顶点着色器纹理textureIn的location编号
                                GL_FLOAT,//同上
                                reinterpret_cast<const void *>(2 * sizeof(float)), //坐标数组的起始位置
                                2,//同上
                                4 * sizeof(float));//同上
    m_shader->enableAttributeArray(textureIn_num); //同上

//    GL_CHECK(glVertexAttribPointer(textureIn_num, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
//                     reinterpret_cast<const void *>(2 * sizeof(float))));
//    GL_CHECK(glEnableVertexAttribArray(textureIn_num));

    //分配纹理(材质)内存空间,并设置参数,并分配显存空间
    m_textureYUV.resize(3);
    for (int i {}; auto &item : m_textureYUV) {

        item.reset(new QOpenGLTexture(QOpenGLTexture::Target2D));
        const auto w{i ? m_w / 2 : m_w},h {i ? m_h / 2 : m_h};
        item->setSize(w,h);
        item->setFormat(QOpenGLTexture::R8_UNorm);
        /*
            R8：表示纹理中的每个像素都有一个 8 位的红色通道。没有绿色、蓝色或 alpha 通道。
            UNorm：表示无符号归一化（Unsigned Normalized），
            归一化的意思是纹理数据会映射到 [0.0, 1.0] 范围，
            对于 8 位无符号整数，0 映射到 0.0，255 映射到 1.0。
        */
        item->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
        /*放大过滤,线性插值*/

        item->setWrapMode(QOpenGLTexture::ClampToEdge);
        /*表示当纹理坐标超出[0.0, 1.0] 范围时,将纹理坐标钳制到边缘值*/
        item->allocateStorage();//分配显存
        ++i;
        qDebug() << "textureId: "  << item->textureId();
    }

//    for (int i {}; i < std::size(m_textureYUV); ++i) {
//        m_textureYUV[i] = new QOpenGLTexture(QOpenGLTexture::Target2D);
//        if (!i){
//            m_textureYUV[i]->setSize(m_w,m_h);
//        }else{
//            m_textureYUV[i]->setSize(m_w / 2,m_h / 2);
//        }
//        m_textureYUV[i]->setFormat(QOpenGLTexture::R8_UNorm);
//        /*
//            R8：表示纹理中的每个像素都有一个 8 位的红色通道。没有绿色、蓝色或 alpha 通道。
//            UNorm：表示无符号归一化（Unsigned Normalized），
//            归一化的意思是纹理数据会映射到 [0.0, 1.0] 范围，
//            对于 8 位无符号整数，0 映射到 0.0，255 映射到 1.0。
//         */
//
//        m_textureYUV[i]->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
//        //放大过滤,线性插值
//        m_textureYUV[i]->setWrapMode(QOpenGLTexture::ClampToEdge);
//        //QOpenGLTexture::ClampToEdge 是一种纹理环绕模式（wrap mode），
//        // 表示当纹理坐标超出 [0.0, 1.0] 范围时，将纹理坐标钳制到边缘值
//        m_textureYUV[i]->allocateStorage();//分配显存
//        qDebug() << "textureId: " << m_textureYUV[i]->textureId();
//    }

    //关联片元着色器的uniform sampler2D(均匀采样器2D)
    m_shader->setUniformValue("tex_y",0);
    m_shader->setUniformValue("tex_u",1);
    m_shader->setUniformValue("tex_v",2);

    //m_VAO->release();//被QOpenGLVertexArrayObject::Binder vao(m_VAO);取代
    m_EBO->release();
    m_VBO->release();
    m_shader->release();

    void (XVideoWidget::*f)(){&XVideoWidget::update};
    connect(&timer,&QTimer::timeout,this,f);
    timer.start(40);
    qDebug() << "end " << __FUNCTION__ ;
}

void XVideoWidget::paintGL() {

    qDebug() << "begin: " << __FUNCTION__;

    if (!m_file.isOpen()){
        qDebug() << "m_file is not open";
        return;
    }

    m_shader->bind();
    QOpenGLVertexArrayObject::Binder vao(m_VAO.get());
    //m_VAO->bind(); //被QOpenGLVertexArrayObject::Binder vao(m_VAO);取代
    m_VBO->bind();
    m_EBO->bind();

    if (m_file.atEnd()){
        m_file.seek(0);
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);//用于支持竖屏

    for(int i{};auto &item: m_textureYUV){
        //Y数据的长度m_w * m_h
        //U数据的长度 m_w * m_h / 4
        //V数据的长度 m_w * m_h / 4
        const auto len{i ? m_w * m_h / 4 : m_w * m_h};
        item->bind(i);
        item->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, m_file.read(len));
        ++i;
    }

    GL_CHECK(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
    //GL_CHECK(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

    for (auto &item:m_textureYUV) {
        item->release();
    }

    m_shader->release();
    //m_VAO->release(); //被QOpenGLVertexArrayObject::Binder vao(m_VAO);取代
    m_VBO->release();
    m_EBO->release();

    qDebug() << "end: " << __FUNCTION__;
}

void XVideoWidget::resizeGL(int w, int h) {
    qDebug() << __FUNCTION__ << " w:" << w << " h:" << h;
    //glViewport(0, 0, w, h);
}

void XVideoWidget::checkOpenGLError(const char* stmt, const char* fname,const int &line) {
    const auto err{glGetError()};
    if(GL_NO_ERROR != err) {
        qDebug() << "OpenGL error " << err << " at " << fname << ":" << line << " - for " << stmt;
    }
}

void XVideoWidget::cleanup() {
    makeCurrent();
    for(auto &item:m_textureYUV){
        if (item){
            item->release();
            item->destroy();
        }
    }

    if (m_shader){
        m_shader->release();
    }

    if (m_VAO){
        m_VAO->release();
        m_VAO->destroy();
    }

    if (m_VBO){
        m_VBO->release();
        m_VBO->destroy();
    }

    if (m_EBO){
        m_EBO->release();
        m_EBO->destroy();
    }
    doneCurrent();
}
