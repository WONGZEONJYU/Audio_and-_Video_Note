//
// Created by Fy-WONG on 2024/10/28.
//

#include "xcamera_widget.hpp"

#include <qevent.h>
#include <QListWidget>
#include <QStyleOption>
#include <QPainter>

XCameraWidget::XCameraWidget(QWidget *parent) :
QWidget(parent) {
    setAcceptDrops(true);
}

void XCameraWidget::dragEnterEvent(QDragEnterEvent *event) {
    //接受拖拽进入
    event->acceptProposedAction();
    QWidget::dragEnterEvent(event);
}

void XCameraWidget::dropEvent(QDropEvent *event) {
    //qDebug() << event->source()->objectName();

    const auto wid{dynamic_cast<QListWidget*>(event->source())};
    qDebug() << wid->currentRow();

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
