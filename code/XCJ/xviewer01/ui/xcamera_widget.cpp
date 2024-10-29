//
// Created by Fy-WONG on 2024/10/28.
//

#include "xcamera_widget.hpp"
#include <qevent.h>
#include <QListWidget>
#include <QStyleOption>
#include <QPainter>
#include <xdecodetask.hpp>
#include <xdemuxtask.hpp>
#include <xvideo_view.hpp>
#include "xcamera_config.hpp"
#include <QWindow>

XCameraWidget::XCameraWidget(QWidget *parent) :
QWidget(parent) {
    setAcceptDrops(true);
}

void XCameraWidget::dragEnterEvent(QDragEnterEvent *event) {
    //接受拖拽进入
    event->acceptProposedAction();
    QWidget::dragEnterEvent(event);
}

//松开拖拽
void XCameraWidget::dropEvent(QDropEvent *event) {
    //qDebug() << event->source()->objectName();

    const auto wid{dynamic_cast<QListWidget*>(event->source())};
    qDebug() << wid->currentRow();
    auto cam{XCamera_Config_()->GetCam(wid->currentRow())};
    //rtsp://admin:123456@192.168.0.123/stream0
    Open(cam.m_sub_url);
    QWidget::dropEvent(event);
}

void XCameraWidget::paintEvent(QPaintEvent *event) {

    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt,
        &painter, this);
    QWidget::paintEvent(event);
}

bool XCameraWidget::Open(const QString &url){

    if (!m_demux_){
        TRY_CATCH(CHECK_EXC(m_demux_.reset(new XDemuxTask())),return {});
    }else {
        m_demux_->Stop();
    }

    if (!m_decode_){
        TRY_CATCH(CHECK_EXC(m_decode_.reset(new XDecodeTask())),return {});
    }else {
        m_decode_->Stop();
    }
    if (!m_view_){
        TRY_CATCH(CHECK_EXC(m_view_.reset(XVideoView::create())),return {});
    }
    //打开解封转
    IS_FALSE_(m_demux_->Open(url.toStdString()),return {});

    XCodecParameters_sp parm;
    IS_FALSE_((parm = m_demux_->CopyVideoParm()).operator bool(),return {});
    //打开视频解码器
    IS_FALSE_(m_decode_->Open(parm),return {});

    //设定解码线程接收解封转数据
    m_demux_->set_next(m_decode_.get());

    //初始化渲染
    const auto wid{QWindow::fromWinId(winId())};
    m_view_->Set_Win_ID(reinterpret_cast<void *>(wid->winId()));

    m_view_->Init(*parm);

    m_demux_->Start();
    m_decode_->Start();

    return true;
}

void XCameraWidget::Draw() const {
    if (!m_demux_ || !m_decode_ || !m_view_){
        return;
    }
    // const auto f{m_decode_->CopyFrame()};
    // if (!f){
    //     return;
    // }

    if (const auto f{m_decode_->CopyFrame()}) {
        m_view_->DrawFrame(*f);
    }
}
