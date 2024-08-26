//
// Created by wong on 2024/8/26.
//

// You may need to build the project (run Qt uic code generator) to get "ui_TestRGB.h" resolved

#include "testrgb.hpp"
#include "ui_testrgb.h"
#include <QPainter>
#include <QImage>

static inline constexpr auto Width{800},Height{600};

TestRGB::TestRGB(QWidget *parent) :
        QWidget(parent), ui(new Ui::TestRGB) {
    ui->setupUi(this);
}

TestRGB::~TestRGB() {
    delete ui;
}

void TestRGB::paintEvent(QPaintEvent *e) {

    QImage image(Width,Height,QImage::Format_RGB888);
    QPainter painter;
    painter.begin(this);

    auto dst_{image.bits()};
    uint8_t r{0xff};
    for (uint32_t h {}; h < Height; ++h,--r) {
        const auto drift{ h * Width * 3 };
        for (uint32_t w {}; w < Width * 3; w += 3) { //Width * 3 宽度的像素是一个rgb刚好24bit
            dst_[w + drift] = r;
            dst_[w + drift + 1] = 0;
            dst_[w + drift + 2] = 0;
        }
    }

    painter.drawImage(0,0,image);
    painter.end();
    //QWidget::paintEvent(e);
}
