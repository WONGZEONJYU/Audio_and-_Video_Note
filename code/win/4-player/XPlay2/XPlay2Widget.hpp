//
// Created by Administrator on 2024/7/27.
//

#ifndef XPLAY2_XPLAY2WIDGET_HPP
#define XPLAY2_XPLAY2WIDGET_HPP

#include <QWidget>
#include <QSharedPointer>
#include "XVideoWidget.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class XPlay2Widget; }
QT_END_NAMESPACE

class XPlay2Widget;
class XDemuxThread;
using XPlay2Widget_sptr = QSharedPointer<XPlay2Widget>;

class XPlay2Widget final : public QWidget {
Q_OBJECT
    explicit XPlay2Widget(QWidget *parent = nullptr);
    void Construct() noexcept(false);
    void DeConstruct() noexcept;
    void timerEvent(QTimerEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void SetPause(const bool &);

public:
    static XPlay2Widget_sptr Handle() noexcept(false);
    ~XPlay2Widget() override;

private slots:
    void OpenFile();
    void OpenURL();
    void PlayOrPause();
    void SliderPressed();
    void SliderReleased();
    void VolumeReleased();
private:
    std::atomic_bool m_is_SliderPress{};
    QSharedPointer<Ui::XPlay2Widget> m_ui;
    QSharedPointer<XDemuxThread> m_dmt;
};

#endif
