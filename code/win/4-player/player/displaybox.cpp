//
// Created by wong on 2024/6/22.
//

// You may need to build the project (run Qt uic code generator) to get "ui_DisplayBox.h" resolved

#include "displaybox.hpp"
#include "ui_displaybox.h"
#include <QPainter>

DisplayBox::DisplayBox(QWidget *parent) :
        QWidget(parent), ui(new Ui::DisplayBox) {
    ui->setupUi(this);
}

DisplayBox::~DisplayBox() {
    delete ui;
}

void DisplayBox::paintEvent(QPaintEvent *event) {

    QPainter painter(this);
    painter.setBrush(Qt::black);
    painter.drawRect(0, 0, width(), height()); //先画成黑色

    if (m_frame.size().width() <= 0) {
        return;
    }

    ///将图像按比例缩放成和窗口一样大小
    auto vp {std::move(m_frame.scaled(size(),Qt::KeepAspectRatio))};

    auto x{(width() - vp.width()) / 2};
    auto y{(height() - vp.height()) / 2};

//    x /= 2;
//    y /= 2;

    painter.drawImage(QPoint(x,y),vp); //画出图像

    //QWidget::paintEvent(event);
}

void DisplayBox::slot_get_one_frame(QImage &vp) {
    m_frame = std::move(vp);
    update();
}

