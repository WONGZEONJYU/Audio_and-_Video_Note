//
// Created by Administrator on 2024/7/27.
//

#include "XPlay2Widget.hpp"
#include "ui_XPlay2Widget.h"
#include "XHelper.hpp"
#include <QFileDialog>
#include <XDemuxThread.hpp>

XPlay2Widget_sptr XPlay2Widget::Handle() noexcept(false){

    XPlay2Widget_sptr obj;
    CHECK_EXC(obj.reset(new XPlay2Widget()));
    CHECK_EXC(obj->Construct(),obj.reset());
    return obj;
}

XPlay2Widget::XPlay2Widget(QWidget *parent) :
        QWidget(parent) {
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
    } catch (const std::exception &e) {
        m_ui->isPlay->setEnabled(true);
        qDebug() << e.what();
        throw;
    }
}

void XPlay2Widget::timerEvent(QTimerEvent *event) {
    const auto total {m_dmt->totalMS()};
    if (total > 0) {
        const auto pos {static_cast<double>(m_dmt->Pts()) / static_cast<double>(total)};
        const auto v{static_cast<decltype(pos)>(m_ui->PlayPos->maximum()) * pos};
        m_ui->PlayPos->setValue(static_cast<int>(v));
    }
}

void XPlay2Widget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    //m_ui->VideoWidget->resize(size());
}

void XPlay2Widget::mouseDoubleClickEvent(QMouseEvent *event) {
    //QWidget::mouseDoubleClickEvent(event);
    isFullScreen() ? showNormal(): showFullScreen();
}

void XPlay2Widget::SetPause(const bool &b) {
    b ? m_ui->isPlay->setText("Play") : m_ui->isPlay->setText("Pause");
}

void XPlay2Widget::PlayOrPause() {
    sender()->blockSignals(true);
    qDebug() << __func__ ;
    const auto b {!m_dmt->is_Pause()};
    SetPause(b);
    m_dmt->SetPause(b);
    sender()->blockSignals(false);
}
