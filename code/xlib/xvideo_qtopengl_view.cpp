#ifdef MACOS
#if 0
#include "xvideo_qtopengl_view.hpp"
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>
#include <algorithm>
#include "xavframe.hpp"
#include "xcodec_parameters.hpp"

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
static inline constexpr GLfloat ver_tex_coordinate[]{
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
    QMutexLocker locker(&m_mux_);
    cleanup();
}

#define Separator "*********************************************"

//初始化opengl

void XVideoWidget::initializeGL() {

    qDebug() << "\n" << Separator << "begin " << __func__ << Separator;

    QMutexLocker locker(&m_mux_);
    //初始化opengl
    initializeOpenGLFunctions();

    qDebug() << "OpenGL version: " << reinterpret_cast<const char*>(glGetString(GL_VERSION));
    qDebug() << "OpenGL GLSL version: " <<  reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));

    //program加载shader(顶点和片元)脚本
    TRY_CATCH(CHECK_EXC(m_shader_.reset(new QOpenGLShaderProgram(this))),return);

    //m_shader->create(); //通常不需要手动调用,因为 addShader, link, 等函数会隐式创建。
    //顶点shader
    CHECK_FALSE_(m_shader_->addShaderFromSourceCode(QOpenGLShader::Vertex,Vertex_shader));
    qDebug() << m_shader_->log();

    //片元shader
    CHECK_FALSE_(m_shader_->addShaderFromSourceCode(QOpenGLShader::Fragment,Fragment_shader));
    qDebug() << m_shader_->log();

    //编译shader
    CHECK_FALSE_(m_shader_->link());
    qDebug() << m_shader_->log();
    //绑定shader
    CHECK_FALSE_(m_shader_->bind());
    qDebug() << m_shader_->log();

    // VAO VBO EBO详情看链接到解释
    //https://blog.csdn.net/weixin_44179561/article/details/124275761
    //https://learnopengl-cn.readthedocs.io/zh/latest/01%20Getting%20started/05%20Shaders/

    TRY_CATCH(CHECK_EXC(m_VAO_.reset(new QOpenGLVertexArrayObject(this))),return);
    const QOpenGLVertexArrayObject::Binder vao(m_VAO_.get());
    //采用RAII技术代替m_VAO->create(); m_VAO->bind();m_VAO->release();

    TRY_CATCH(CHECK_EXC(m_VBO_.reset(new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer))),return);
    m_VBO_->create();
    CHECK_FALSE_(m_VBO_->bind());
    m_VBO_->allocate(ver_tex_coordinate, sizeof(ver_tex_coordinate));
    m_VBO_->setUsagePattern(QOpenGLBuffer::StaticDraw);

    TRY_CATCH(CHECK_EXC(m_EBO_.reset(new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer))),return);
    m_EBO_->create();
    CHECK_FALSE_(m_EBO_->bind());
    m_EBO_->allocate(direction, sizeof(direction));

    //传递顶点和材质坐标
    const auto vertexIn_num{m_shader_->attributeLocation(GET_STR(vertexIn))};
    qDebug() << "vertexIn_num = " << vertexIn_num;
    //顶点坐标设置
    m_shader_->setAttributeArray(vertexIn_num, //顶点着色器vertexIn的location编号
                                GL_FLOAT, //坐标类型
                                nullptr, //坐标数组的起始位置
                                2, //坐标数量,例如(x,y)
                                4 * sizeof(float)); //访问偏移
    m_shader_->enableAttributeArray(vertexIn_num); //启用顶点坐标

#if 0
    GL_CHECK(glVertexAttribPointer(vertexIn_num, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float ), nullptr));
    GL_CHECK(glEnableVertexAttribArray(vertexIn_num));
#endif

    const auto textureIn_num{m_shader_->attributeLocation(GET_STR(textureIn))};
    qDebug() << "textureIn_num = " << textureIn_num;

    //纹理(材质)坐标设置
    m_shader_->setAttributeArray(textureIn_num, //顶点着色器纹理textureIn的location编号
                                GL_FLOAT,//同上
                                reinterpret_cast<const void *>(2 * sizeof(float)), //坐标数组的起始位置
                                2,//同上
                                4 * sizeof(float));//同上
    m_shader_->enableAttributeArray(textureIn_num); //同上

#if 0
    GL_CHECK(glVertexAttribPointer(textureIn_num, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                     reinterpret_cast<const void *>(2 * sizeof(float))));
    GL_CHECK(glEnableVertexAttribArray(textureIn_num));
#endif

    //关联片元着色器的uniform sampler2D(均匀采样器2D)
#if 1
    constexpr const char *tex_[]{GET_STR(tex_y), GET_STR(tex_u), GET_STR(tex_v)};
    for(int i{};const auto &item:tex_){
        m_shader_->setUniformValue(item,i++);
    }
#else
    m_shader_->setUniformValue("tex_y",0);
    m_shader_->setUniformValue("tex_u",1);
    m_shader_->setUniformValue("tex_v",2);
#endif
    //m_VAO->release();//被QOpenGLVertexArrayObject::Binder vao(m_VAO);取代
    m_EBO_->release();
    m_VBO_->release();
    m_shader_->release();

    qDebug() << "\n" << Separator << "end " << __func__ << Separator;
}

void XVideoWidget::paintGL() {

   // qDebug() << "begin " << __FUNCTION__ ;
    QMutexLocker locker(&m_mux_);

    CHECK_FALSE_(!m_yuv_datum_.isEmpty(),return);

    for (const auto &item : m_yuv_datum_){
        CHECK_FALSE_(!item.isEmpty(),return);
    }

    const QOpenGLVertexArrayObject::Binder vao(m_VAO_.get());
    //m_VAO->bind(); //被QOpenGLVertexArrayObject::Binder vao(m_VAO);取代

    const XRAII r([this]{
        CHECK_FALSE_(m_shader_->bind());
        CHECK_FALSE_(m_VBO_->bind());
        CHECK_FALSE_(m_EBO_->bind());
        },[this]{
        m_shader_->release();
        //m_VAO->release(); //被QOpenGLVertexArrayObject::Binder vao(m_VAO);取代
        m_VBO_->release();
        m_EBO_->release();
    });

    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
    GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));//用于支持竖屏

    for(uint32_t i{};const auto &item:m_textureYUV_) {
        //Y数据的长度m_w * m_h
        //U数据的长度 m_w * m_h / 4
        //V数据的长度 m_w * m_h / 4
        item->bind(i);
        item->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8,
                      m_yuv_datum_[i++].data());
    }
#if 1
    GL_CHECK(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
#else
    GL_CHECK(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
#endif
    for (const auto &item:m_textureYUV_) {
        item->release();
    }

   // qDebug() << "end: " << __FUNCTION__;
}

void XVideoWidget::resizeGL(const int w,const int h) {

    //QMutexLocker locker(&m_mux_);
    GL_CHECK(glViewport(0, 0, w, h));
}

void XVideoWidget::cleanup() noexcept(true) {

    makeCurrent();

    m_yuv_datum_.clear();

    for(auto &item:m_textureYUV_){
        if (item){
            item->release();
            item->destroy();
        }
    }
    m_textureYUV_.clear();

    if (m_shader_){
        m_shader_->release();
    }
    m_shader_.reset();

    if (m_VAO_){
        m_VAO_->release();
        m_VAO_->destroy();
    }
    m_VAO_.reset();

    if (m_VBO_){
        m_VBO_->release();
        m_VBO_->destroy();
    }
    m_VBO_.reset();

    if (m_EBO_){
        m_EBO_->release();
        m_EBO_->destroy();
    }
    m_EBO_.reset();

    doneCurrent();
}

bool XVideoWidget::Init(const int &w,const int&h) noexcept(true) {

    QMutexLocker locker(&m_mux_);

    CHECK_FALSE_(m_shader_ && m_VBO_ && m_EBO_ && m_VAO_,
        PRINT_ERR_TIPS(GET_STR(Please call the show() function first));
        return {});

    const QOpenGLVertexArrayObject::Binder vao(m_VAO_.get());

    const XRAII r([this]{
        CHECK_FALSE_(m_shader_->bind());
        CHECK_FALSE_(m_VBO_->bind());
        CHECK_FALSE_(m_EBO_->bind());
        },[this]{
            m_shader_->release();
            m_VBO_->release();
            m_EBO_->release();
    });

    /*分配纹理(材质)内存空间,并设置参数,并分配显存空间*/
    m_yuv_datum_.clear();
    m_yuv_datum_.resize(3);

    m_textureYUV_.clear(); //分配前，需要先释放
    m_textureYUV_.resize(3);

    for(int i {};auto &item:m_textureYUV_) {

        TRY_CATCH(CHECK_EXC(item.reset(new QOpenGLTexture(QOpenGLTexture::Target2D))),
                  cleanup();return {});

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
    m_w_ = w,m_h_ = h;
    m_half_w_ = w / 2,m_half_h_ = h / 2;

    return true;
}

bool XVideoWidget::Init(const XCodecParameters &parameters) noexcept(true) {
    return Init(parameters.Width(), parameters.Height());
}

void XVideoWidget::Repaint(const XAVFrame &frame) {

    CHECK_FALSE_(frame.data[0] && frame.width * frame.height,
        PRINT_ERR_TIPS(GET_STR(Non-video frame!));return);

    CHECK_FALSE_(frame.width == m_w_ && frame.height == m_h_,
              PRINT_ERR_TIPS(GET_STR(Resolution error));return);

    {
        QMutexLocker locker(&m_mux_);

        CHECK_FALSE_(m_shader_ && m_VAO_ && m_VBO_ && m_EBO_ &&
            !m_yuv_datum_.isEmpty() && !m_textureYUV_.isEmpty()
            ,PRINT_ERR_TIPS(GET_STR(Please call the Init() function first ));
            return);

        if (m_w_ != frame.linesize[0]){ //需对齐
            copy_y(frame);
            copy_uv(frame);
        }else{ //无需对齐
            for (uint32_t i{};auto &item:m_yuv_datum_) {
                const auto len{ i ? m_half_w_ * m_half_h_ : m_w_ * m_h_};
                const auto src{frame.data[i++]};
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

void XVideoWidget::copy_y(const XAVFrame &frame){

    if (const auto len{m_h_ * frame.linesize[0]}; m_yuv_datum_[0].capacity() <= len){
        m_yuv_datum_[0].clear();
        m_yuv_datum_[0].resize(len + (len >> 1));
    }

    for(uint32_t i{};i < m_h_;++i){
        const auto src{frame.data[0] + frame.linesize[0] * i};
        const auto dst{m_yuv_datum_[0].data() + m_w_ * i};
        std::copy_n(src,m_w_,dst);
    }
}

void XVideoWidget::copy_uv(const XAVFrame &frame){

    for (auto n{1}; n <=2 ; ++n) {
        if (const auto len{m_half_h_ * frame.linesize[n]}; m_yuv_datum_[n].capacity() <= len) {
            m_yuv_datum_[n].clear();
            m_yuv_datum_[n].resize(len + (len >> 1));
        }

        for (uint32_t i{}; i < m_half_h_ ; ++i) {
            const auto src{frame.data[n] + frame.linesize[n] * i};
            const auto dst{m_yuv_datum_[n].data() + m_half_w_ * i};
            std::copy_n(src,m_half_w_,dst);
        }
    }
}

#endif
#endif
