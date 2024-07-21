//
// Created by wong on 2024/7/21.
//

#include "XVideoWidget.hpp"
//自动加双引号
#define GET_STR(args) #args

//顶点shader
static inline constexpr auto vString{
GET_STR(
    attribute vec4 vertexIn; //顶点坐标
    attribute vec2 textureIn; //材质坐标
    varying vec2 textureOut;
    void main(void){
        gl_Position = vertexIn;
        textureOut = textureIn;
    }
)};

//片元shader

static inline constexpr auto tString{
GET_STR(
    varying vec2 textureOut;

)};

XVideoWidget::XVideoWidget(QWidget *parent):
    QOpenGLWidget(parent){
}

//XVideoWidget::~XVideoWidget() {
//
//}

//初始化opengl
void XVideoWidget::initializeGL() {

    qDebug() << __FUNCTION__;
    //初始化opengl
    initializeOpenGLFunctions();


    //program加载shader(顶点和片元)脚本
    //片元(像素)
 //   program.addShaderFromSourceCode(QOpenGLShader::Fragment);
    //顶点shader
   // program.addShaderFromSourceCode(QOpenGLShader::Vertex);


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
