// You may need to build the project (run Qt uic code generator) to get "ui_XPlayVieo.h" resolved

#include "xplayvideo.hpp"
#include <xvideo_view.hpp>
#include "ui_xplayvideo.h"

void XPlayVideo::timerEvent(QTimerEvent *const event) {

#ifdef MACOS
    m_player_.Update([this](const XAVFrame &frame) {
        Repaint(frame);
    });
#else
    m_player_.Update();
#endif
    QObject::timerEvent(event);
}

void XPlayVideo::closeEvent(QCloseEvent * const event) {
#ifdef MACOS
    QWidget::closeEvent(event);
#else
    QDialog::closeEvent(event);
#endif
}

XPlayVideo::XPlayVideo(QWidget *parent) :
#ifdef MACOS
XVideoWidget(parent),
#else
QDialog(parent),
#endif
m_ui_(new Ui::XPlayVideo) {
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
    show();
    Init(*m_player_.get_video_params());
#else
    if (!m_player_.Open(url.toStdString(),reinterpret_cast<void*>(m_ui_->widget->winId()))) {
        return false;
    }
#endif
    m_player_.Start();
    startTimer(1);
    return true;
}

void XPlayVideo::Close() {
    m_player_.Stop();
}

void XPlayVideo::SetSpeed() {
    const auto s{m_ui_->speed->value()};
    const auto speed{s >=0 ? static_cast<float>(s) / 10.0f : static_cast<float>(s- 9) };
    m_ui_->speedtxt->setText(QString::number(speed));
    m_player_.SetSpeed(speed);
}

#ifdef MACOS
int XPlayVideo::exec() {
    QEventLoop loop;
    (void )connect(this,&QWidget::destroyed,&loop,&QEventLoop::quit);
    return loop.exec();
}
#endif
