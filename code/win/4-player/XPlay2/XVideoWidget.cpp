//
// Created by Administrator on 2024/8/1.
//

#include "XVideoWidget.hpp"
#include <QByteArray>
#include <algorithm>
#include "XAVFrame.hpp"

XVideoWidget::XVideoWidget(QWidget *parent):QOpenGLWidget(parent){
#if defined(__APPLE__) && defined(__MACH__)
    QSurfaceFormat format;
    format.setVersion(4, 1);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);
    setFormat(format);
#endif
}

XVideoWidget::~XVideoWidget() {
    cleanup();
}

//初始化opengl
void XVideoWidget::initializeGL() {

    qDebug() << "begin " << __FUNCTION__ ;
    QMutexLocker locker(&m_mux);
    //初始化opengl
    initializeOpenGLFunctions();

    qDebug() << "OpenGL version: " << reinterpret_cast<const char*>(glGetString(GL_VERSION));
    qDebug() << "OpenGL GLSL version: " <<  reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));

    //program加载shader(顶点和片元)脚本
    //m_shader = new QOpenGLShaderProgram(this);
    m_shader.reset(new QOpenGLShaderProgram(this));

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

    // VAO VBO EBO详情看链接到解释
    // https://blog.csdn.net/weixin_44179561/article/details/124275761
    //https://learnopengl-cn.readthedocs.io/zh/latest/01%20Getting%20started/05%20Shaders/
    m_VAO.reset(new QOpenGLVertexArrayObject(this));
//    m_VAO->create();
//    m_VAO->bind();
    QOpenGLVertexArrayObject::Binder vao(m_VAO.get());//代替m_VAO->create(); m_VAO->bind();m_VAO->release();
    //采用RAII技术

    m_VBO.reset(new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer));
    m_VBO->create();
    qDebug() << "m_VBO->bind() = " << m_VBO->bind();
    m_VBO->allocate(ver_tex_coordinate, sizeof(ver_tex_coordinate));
    m_VBO->setUsagePattern(QOpenGLBuffer::StaticDraw);

    m_EBO.reset(new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer));
    m_EBO->create();
    qDebug() << "m_EBO->bind() = " << m_EBO->bind();
    m_EBO->allocate(direction, sizeof(direction));

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

    //关联片元着色器的uniform sampler2D(均匀采样器2D)
    m_shader->setUniformValue("tex_y",0);
    m_shader->setUniformValue("tex_u",1);
    m_shader->setUniformValue("tex_v",2);

    //m_VAO->release();//被QOpenGLVertexArrayObject::Binder vao(m_VAO);取代
    m_EBO->release();
    m_VBO->release();
    m_shader->release();

    qDebug() << "end " << __FUNCTION__ ;
}

void XVideoWidget::paintGL() {

    QMutexLocker locker(&m_mux);
    if (m_yuv_datum.isEmpty()){
        return;
    }

    for (auto &item : m_yuv_datum){
        if (item.isEmpty()){
            return;
        }
    }

    auto b { m_shader->bind() };
    QOpenGLVertexArrayObject::Binder vao(m_VAO.get());
    //m_VAO->bind(); //被QOpenGLVertexArrayObject::Binder vao(m_VAO);取代
    b = m_VBO->bind();
    b = m_EBO->bind();

    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
    GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));//用于支持竖屏

    for(uint32_t i{};auto &item:m_textureYUV) {
        //Y数据的长度m_w * m_h
        //U数据的长度 m_w * m_h / 4
        //V数据的长度 m_w * m_h / 4
        item->bind(i);
        item->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8,
                      reinterpret_cast<const uint8_t *>(m_yuv_datum[i++].data()));
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

    //qDebug() << "end: " << __FUNCTION__;
}

void XVideoWidget::resizeGL(const int w,const int h) {

    //QMutexLocker locker(&m_mux);
    GL_CHECK(glViewport(0, 0, w, h));
}

void XVideoWidget::cleanup() noexcept(true) {

    makeCurrent();

    m_yuv_datum.clear();

    for(auto &item:m_textureYUV){
        if (item){
            item->release();
            item->destroy();
        }
    }
    m_textureYUV.clear();

    if (m_shader){
        m_shader->release();
    }
    m_shader.reset();

    if (m_VAO){
        m_VAO->release();
        m_VAO->destroy();
    }
    m_VAO.reset();

    if (m_VBO){
        m_VBO->release();
        m_VBO->destroy();
    }
    m_VBO.reset();

    if (m_EBO){
        m_EBO->release();
        m_EBO->destroy();
    }
    m_EBO.reset();

    doneCurrent();
}

void XVideoWidget::Init(const int &w,const int&h) noexcept(false) {

    QMutexLocker locker(&m_mux);

    if (!m_shader || !m_VBO || !m_EBO || !m_VAO){
        qDebug() << "Please call the show() function first";
        return;
    }

    QOpenGLVertexArrayObject::Binder vao(m_VAO.get());

    XRAII r([this]{

        auto b{ m_shader->bind() };
        b = m_VBO->bind();
        b = m_EBO->bind();
        },[this]{
        m_shader->release();
        m_VBO->release();
        m_EBO->release();
    });

    /*分配纹理(材质)内存空间,并设置参数,并分配显存空间*/
    try {
        m_yuv_datum.clear();
        m_yuv_datum.resize(3);

        m_textureYUV.clear(); //分配前，需要先释放
        m_textureYUV.resize(3);

        for(int i {};auto &item : m_textureYUV){
            CHECK_EXC(item.reset(new QOpenGLTexture(QOpenGLTexture::Target2D)),locker.unlock());

            const auto _w{i ? w / 2 : w},_h {i ? h / 2 : h};
            item->setSize(_w,_h);

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
            item->allocateStorage();
            /*分配显存*/
            //qDebug() << "textureId: "  << item->textureId();
            ++i;
        }
        m_w = w,m_h = h;
        m_half_w = w / 2,m_half_h = h / 2;
    } catch (...) {
        r.destroy();
        vao.release();
        cleanup();
        locker.unlock();
        throw;
    }
}

void XVideoWidget::Repaint(const XAVFrame_sptr &frame) {

    if (!frame){
        qDebug() << __func__ << "XAVFrame_sptr is empty";
        return;
    }

    if (!(frame->width * frame->height)){
        qDebug() << __func__ << " Non-video frames";
        return;
    }

    {
        QMutexLocker locker(&m_mux);

        if (!m_shader || !m_VAO || !m_VBO || !m_EBO || m_yuv_datum.isEmpty() || m_textureYUV.isEmpty()) {
            qDebug() << "Please call the Init() function first ";
            return;
        }

        if (frame->width != m_w || frame->height != m_h) {
            qDebug() << __func__ << " Resolution error";
            return;
        }

        if (m_w != frame->linesize[0]){ //需对齐
            copy_y(frame);
            copy_uv(frame);
        }else{ //无需对齐

            for (uint32_t i{};auto &item:m_yuv_datum) {
                const auto len{ i ? m_half_w * m_half_h : m_w * m_h};
                const auto src{reinterpret_cast<const char*>(frame->data[i++])};
                if (item.capacity() <= len){
                    item.clear();
                    item.resize(len + (len >> 1));
                }
                std::copy_n(src,len,item.data());
            }
        }
    }

    update();
}

void XVideoWidget::copy_y(const XAVFrame_sptr &frame){

    if (const auto len{m_h * frame->linesize[0]}; m_yuv_datum[0].capacity() <= len){
        m_yuv_datum[0].clear();
        m_yuv_datum[0].resize(len + (len >> 1));
    }

    for(uint32_t i{};i < m_h;++i){
        const auto src{reinterpret_cast<const char*>(frame->data[0] + frame->linesize[0] * i)};
        const auto dst{m_yuv_datum[0].data() + m_w * i};
        std::copy_n(src,m_w.load(),dst);
    }
}

void XVideoWidget::copy_uv(const XAVFrame_sptr &frame){

    for (auto n{1}; n <=2 ; ++n) {
        if (const auto len{m_half_h * frame->linesize[n]}; m_yuv_datum[n].capacity() <= len) {
            m_yuv_datum[n].clear();
            m_yuv_datum[n].resize(len + (len >> 1));
        }

        for (uint32_t i{}; i < m_half_h ; ++i) {
            const auto src{reinterpret_cast<const char*>(frame->data[n] + frame->linesize[n] * i)};
            const auto dst{m_yuv_datum[n].data() + m_half_w * i};
            std::copy_n(src,m_half_w.load(),dst);
        }
    }
}
