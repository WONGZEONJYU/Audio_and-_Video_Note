//
// Created by Administrator on 2024/7/27.
//

#include "XPlay2Widget.hpp"
#include "ui_XPlay2Widget.h"
#include "XHelper.hpp"
#include <QFileDialog>
#include <XDemuxThread.hpp>
#include <QInputDialog>

XPlay2Widget_sptr XPlay2Widget::Handle() noexcept(false) {
    XPlay2Widget_sptr obj;
    CHECK_EXC(obj.reset(new XPlay2Widget()));
    CHECK_EXC(obj->Construct(),obj.reset());
    return obj;
}

XPlay2Widget::XPlay2Widget(QWidget *parent) : QWidget(parent) {
}

XPlay2Widget::~XPlay2Widget() {
    DeConstruct();
}

void XPlay2Widget::Construct() noexcept(false) {

    CHECK_EXC(m_ui.reset(new Ui::XPlay2Widget));
    m_ui->setupUi(this);
    m_dmt.reset(new XDemuxThread());
    m_dmt->Start();
    m_dmt->SetPause(true);
    SetPause(m_dmt->is_Pause());
    startTimer(40);
}

void XPlay2Widget::DeConstruct() noexcept {
    m_dmt.reset();
}

void XPlay2Widget::OpenFile() {
    auto name {QFileDialog::getOpenFileName(this,QString::fromLocal8Bit("Select Media Files"))};
    if (name.isEmpty()){
        return;
    }
    setWindowTitle(name);
    try {
        m_ui->isPlay->setEnabled(false);
        m_dmt->Open(name.toLocal8Bit(),m_ui->VideoWidget);
        if (m_dmt->is_Pause()){
            m_dmt->SetPause(false);
        }
        SetPause(m_dmt->is_Pause());
        m_ui->isPlay->setEnabled(true);
        m_ui->PlayPos->setEnabled(true);
        m_ui->Speed->setEnabled(true);
        m_ui->Speed->setValue(1.0);
        const auto max_v {static_cast<double>(m_ui->VolumeSlider->maximum())};
        qDebug() << "m_dmt->Volume() = " << m_dmt->Volume();
        m_ui->VolumeSlider->setValue(static_cast<int>(m_dmt->Volume() < 0.0 ? m_ui->VolumeSlider->maximum() :
                                                      m_dmt->Volume() * max_v));
    } catch (const std::exception &e) {
        m_ui->isPlay->setEnabled(true);
        qDebug() << e.what();
    }
}

void XPlay2Widget::OpenURL() {
    const auto url{QInputDialog::getText(this,"url","url")};
    if (url.isEmpty()){
        return;
    }

    if (!url.startsWith("rtmp://") &&
        !url.startsWith("rtsp://") &&
        !url.startsWith("sdp:://") &&
        !url.startsWith("rtp://") &&
        !url.startsWith("udp://")) {
        qDebug() << url;
        return;
    }

    try {
        m_ui->isPlay->setEnabled(false);
        m_dmt->Open(url.toLocal8Bit(),m_ui->VideoWidget);
        if (m_dmt->is_Pause()){
            m_dmt->SetPause(false);
        }
        SetPause(m_dmt->is_Pause());
        m_ui->isPlay->setEnabled(true);
        m_ui->PlayPos->setEnabled(false);
        m_ui->PlayPos->setValue(0);
        m_ui->Speed->setEnabled(false);
        m_dmt->SetSpeed(static_cast<float >(m_ui->Speed->value()));
        const auto max_v {static_cast<double >(m_ui->VolumeSlider->maximum())};
        qDebug() << "m_dmt->Volume() = " << m_dmt->Volume();
        m_ui->VolumeSlider->setValue(static_cast<int>(m_dmt->Volume() < 0.0 ? m_ui->VolumeSlider->maximum() :
                                                      m_dmt->Volume() * max_v));
    } catch (const std::exception &e) {
        m_ui->isPlay->setEnabled(true);
        m_ui->PlayPos->setEnabled(true);
        m_ui->Speed->setEnabled(true);
        qDebug() << e.what();
    }
}

void XPlay2Widget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
}

void XPlay2Widget::mouseDoubleClickEvent(QMouseEvent *event) {
    isFullScreen() ? showNormal(): showFullScreen();
}

void XPlay2Widget::SetPause(const bool &b) {
    b ? m_ui->isPlay->setText("Play") : m_ui->isPlay->setText("Pause");
}

void XPlay2Widget::PlayOrPause() {
    sender()->blockSignals(true);
    const auto b {!m_dmt->is_Pause()};
    SetPause(b);
    m_dmt->SetPause(b);
    sender()->blockSignals(false);
}

void XPlay2Widget::timerEvent(QTimerEvent *) {

    if (m_is_SliderPress) {
        return;
    }

    const auto total{m_dmt->totalMS()};

    if (total > 0) {
        const auto pos{static_cast<double>(m_dmt->Pts()) / static_cast<double>(total)},
                    v{static_cast<decltype(pos)>(m_ui->PlayPos->maximum()) * pos};
        m_ui->PlayPos->setValue(static_cast<int>(v));
    }
}

void XPlay2Widget::SliderPressed() {
    m_is_SliderPress = true;
}

void XPlay2Widget::SliderReleased() {

    sender()->blockSignals(true);

    const auto PlayPos{static_cast<double>(m_ui->PlayPos->value())},
                PlayMax{static_cast<double>(m_ui->PlayPos->maximum())};

    const auto pos{PlayPos / PlayMax};

    try {
        m_dmt->Seek(pos);
    } catch (const std::exception &e) {
        qDebug() << e.what();
    }

    m_is_SliderPress = false;

    sender()->blockSignals(false);
}

void XPlay2Widget::VolumeChanged(const int &v) {
    const auto max{static_cast<double >(m_ui->VolumeSlider->maximum())};
    m_dmt->SetVolume(v / max);
}

void XPlay2Widget::SpeedChanged(const double &v){
    m_dmt->SetSpeed(static_cast<float>(v));
}
