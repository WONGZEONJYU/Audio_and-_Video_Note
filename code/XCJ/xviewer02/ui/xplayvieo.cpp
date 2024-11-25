// You may need to build the project (run Qt uic code generator) to get "ui_XPlayVieo.h" resolved

#include "xplayvieo.hpp"
#include <xvideo_view.hpp>
#include "ui_xplayvieo.h"

void XPlayVideo::timerEvent(QTimerEvent *const event) {

#ifdef MACOS
    m_player_.Update([this](const XAVFrame &frame) {
        Repaint(frame);
    });
#else
    m_player_.Update();
#endif
    QWidget::timerEvent(event);
}

void XPlayVideo::closeEvent(QCloseEvent * const event) {
    Close();
    QWidget::closeEvent(event);
}

XPlayVideo::XPlayVideo(QWidget *parent) :
#ifdef MACOS
XVideoWidget(parent),
#else
QWidget(parent),
#endif
m_ui_(new Ui::XPlayVieo) {
    m_ui_->setupUi(this);
}

XPlayVideo::~XPlayVideo() {
    Close();
}

bool XPlayVideo::Open(const QString &url) {

#ifdef MACOS
    if (!m_player_.Open(url.toStdString(),{},true)) {
        return false;
    }
    Init(*m_player_.get_video_params());
#else
    if (!m_player_.Open(url.toStdString(),reinterpret_cast<void*>(winId()))) {
        return false;
    }
#endif
    m_player_.Start();
    m_timer_id = startTimer(10);
    return true;
}

void XPlayVideo::Close() {
    if (m_timer_id >= 0){
         killTimer(m_timer_id);
         m_timer_id = -1;
    }
    m_player_.Stop();
}
