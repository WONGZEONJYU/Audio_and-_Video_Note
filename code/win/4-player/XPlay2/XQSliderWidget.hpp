//
// Created by Administrator on 2024/8/16.
//

#ifndef XPLAY2_XQSLIDERWIDGET_HPP
#define XPLAY2_XQSLIDERWIDGET_HPP

#include <QSlider>

class QMouseEvent;

class XQSliderWidget : public QSlider{
Q_OBJECT
    void mousePressEvent(QMouseEvent *e) override;
public:
    explicit XQSliderWidget(QWidget * = nullptr);
    ~XQSliderWidget() override = default;
};

#endif
