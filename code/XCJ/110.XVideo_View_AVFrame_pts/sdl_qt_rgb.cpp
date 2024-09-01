//
// Created by wong on 2024/8/27.
//

// You may need to build the project (run Qt uic code generator) to get "ui_sdl_qt_rgb.h" resolved

#include "sdl_qt_rgb.hpp"
#include "ui_sdl_qt_rgb.h"
#include <QMessageBox>
#include <QImage>
#include <QResizeEvent>
#include "xvideo_view.hpp"
#include "XAVFrame.hpp"
#include "XHelper.hpp"
#include <QThread>
#include <QStringList>
#include <QSpinBox>

#undef main

sdl_qt_rgb::sdl_qt_rgb(QWidget *parent) :
        QWidget(parent), ui(new Ui::sdl_qt_rgb) {

    m_yuv_file.setFileName("400_300_25.yuv");
    if(!m_yuv_file.open(QFile::ReadOnly)){
        QMessageBox::warning(this,"","open yuv_file error!");
        return;
    }
    ui->setupUi(this);
    connect(this,&sdl_qt_rgb::ViewS, this,&sdl_qt_rgb::View,Qt::UniqueConnection);

    ui->view_fps->setText("fps: 25");


    m_SpinBox = new QSpinBox(this);
    m_SpinBox->move(200,0);
    m_SpinBox->setValue(25);
    m_SpinBox->setRange(1,300);

    m_sdl_w = 400;
    m_sdl_h = 300;

    m_view = XVideoView::create();
    m_view->Init(m_sdl_w,m_sdl_h,XVideoView::YUV420P,
                 reinterpret_cast<void*>(ui->widget->winId()));

    m_frame = new_XAVFrame();
    m_frame->width = m_sdl_w;
    m_frame->height = m_sdl_h;
    m_frame->format = AV_PIX_FMT_YUV420P;

#if 0
    //此方法不是一个好方法
    m_frame->linesize[0] = m_sdl_w;
    m_frame->linesize[1] = m_sdl_w / 2;
    m_frame->linesize[2] = m_sdl_w / 2;
#endif

    FF_ERR_OUT(av_frame_get_buffer(m_frame.get(),1),return);
    m_th = std::thread(&sdl_qt_rgb::Main, this);
}

sdl_qt_rgb::~sdl_qt_rgb() {

    m_th_is_exit = true;
    m_th.join();
    delete ui;
    delete m_view;
    m_yuv_file.close();
}

void sdl_qt_rgb::timerEvent(QTimerEvent *e) {

#if 0
    if (m_yuv_file.atEnd()){
        killTimer(e->timerId());
        QMessageBox::warning(this,"","yuv_file end!");
        return;
    }

    if (m_view->Is_Exit_Window()){
        m_view->Close();
        close();
        return;
    }

    m_yuv_file.read(reinterpret_cast<char *>(m_frame->data[0]),m_sdl_w * m_sdl_h);
    m_yuv_file.read(reinterpret_cast<char *>(m_frame->data[1]),m_sdl_w * m_sdl_h / 4);
    m_yuv_file.read(reinterpret_cast<char *>(m_frame->data[2]),m_sdl_w * m_sdl_h / 4);

    m_view->DrawFrame(m_frame);
#endif
}

void sdl_qt_rgb::resizeEvent(QResizeEvent *e) {
    ui->widget->resize(size());
    ui->widget->move({});
    if (m_view){
        m_view->Scale(width(),height());
    }
    QWidget::resizeEvent(e);
}

void sdl_qt_rgb::Main() {

    while (!m_th_is_exit){
        ViewS();
        if (m_fps > 0){
            XHelper::MSleep(1000 / m_fps);
        } else{
            XHelper::MSleep(10);
        }
    }
}

void sdl_qt_rgb::View(){

    if (m_th_is_exit){
        return;
    }

    if (m_yuv_file.atEnd()){
        m_yuv_file.seek(0);
        return;
    }

    if (m_view->Is_Exit_Window()){
        m_view->Close();
        return;
    }

    m_yuv_file.read(reinterpret_cast<char *>(m_frame->data[0]),m_sdl_w * m_sdl_h);
    m_yuv_file.read(reinterpret_cast<char *>(m_frame->data[1]),m_sdl_w * m_sdl_h / 4);
    m_yuv_file.read(reinterpret_cast<char *>(m_frame->data[2]),m_sdl_w * m_sdl_h / 4);

    m_view->DrawFrame(m_frame);

    QStringList ss;
    ss << "fps: " << QString::number(m_view->view_fps());

    ui->view_fps->setText(ss.join(""));
    m_fps = m_SpinBox->value();

    ui->view_fps->raise();//macOS窗口层次问题
    m_SpinBox->raise();//macOS窗口层次问题
}
