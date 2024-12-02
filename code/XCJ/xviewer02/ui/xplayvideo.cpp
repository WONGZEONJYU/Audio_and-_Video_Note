// You may need to build the project (run Qt uic code generator) to get "ui_XPlayVieo.h" resolved

#include "xplayvideo.hpp"
#include <xvideo_view.hpp>
#include "ui_xplayvideo.h"

void XPlayVideo::timerEvent(QTimerEvent *const event) {

    if (m_is_press_ || m_is_move_ || m_player_.is_pause()){
        m_ui_->pause->setStyleSheet(QString::fromUtf8("background-image: url(:/img/play.png);\n"
                                                      "background-color: rgba(0, 0, 0,0);"));
        return;
    }

    m_ui_->pause->setStyleSheet(QString::fromUtf8("background-image: url(:/img/pause.png);\n"
        "background-color: rgba(0, 0, 0,0);"));

    m_player_.Update();
    m_ui_->pos->setMaximum(static_cast<int>(m_player_.total_ms()));
    m_ui_->pos->setValue(static_cast<int>(m_player_.pos_ms()));
    QObject::timerEvent(event);
}

void XPlayVideo::closeEvent(QCloseEvent * const event) {
    QDialog::closeEvent(event);
}

XPlayVideo::XPlayVideo(QWidget *parent) :QDialog(parent),

m_ui_(new Ui::XPlayVideo) {
    m_ui_->setupUi(this);
}

XPlayVideo::~XPlayVideo() {
    Close();
}

bool XPlayVideo::Open(const QString &url) {

#ifdef MACOS
    if (!m_player_.Open(url.toStdString(),
                        [this](const XAVFrame &frame){
        m_ui_->video->Repaint(frame);
    })) {
        return false;
    }
    show();
    m_ui_->video->Init(*m_player_.get_video_params());
#else
    if (!m_player_.Open(url.toStdString(),
                        reinterpret_cast<void*>(m_ui_->video->winId()))) {
        return false;
    }
#endif
    m_player_.Start();
    m_player_.pause(false);
    startTimer(10);
    m_ui_->pause->setStyleSheet(QString::fromUtf8("background-image: url(:/img/pause.png);\n"
                                                  "background-color: rgba(0, 0, 0,0);"));
    return true;
}

void XPlayVideo::Close() {
    m_player_.Stop();
}

void XPlayVideo::SetSpeed() {
    const auto s{m_ui_->speed->value()};
    const auto speed{s >= 0 ? static_cast<float>(s) / 10.0f : static_cast<float>(s - 9)};
    m_ui_->speedtxt->setText(QString::number(speed));
    m_player_.SetSpeed(speed);
}

void XPlayVideo::Pause() {
    m_player_.pause(!m_player_.is_pause());
}

void XPlayVideo::PlayPos() {
    sender()->blockSignals(true);
    m_player_.Seek(m_ui_->pos->value());
    SetSpeed();
    m_is_move_ = false;
    m_is_press_ = false;
    sender()->blockSignals(false);
}

void XPlayVideo::PlayPosPressed() {
    m_is_press_ = true;
}

void XPlayVideo::Move() {
    m_is_move_ = true;
}
