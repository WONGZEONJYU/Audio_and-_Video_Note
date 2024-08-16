//
// Created by Administrator on 2024/8/16.
//

#include "XQSliderWidget.hpp"
#include <QMouseEvent>

XQSliderWidget::XQSliderWidget(QWidget *p) : QSlider(p){

}

void XQSliderWidget::mousePressEvent(QMouseEvent *e) {
    const auto x {static_cast<double>(e->pos().x())},
            Slider_Len{static_cast<double>(width())};

    const auto pos { (x / Slider_Len) * maximum()};
    setValue(static_cast<int>(pos));
    sliderReleased();
}
