//
// Created by wong on 2024/7/21.
//

// You may need to build the project (run Qt uic code generator) to get "ui_Widget.h" resolved

#include "widget.hpp"
#include "ui_widget.h"

Widget_sptr new_Widget() noexcept(false)
{
    Widget_sptr obj;
    try {
        obj.reset(new Widget());
        obj->Construct();
        return obj;
    } catch (const std::exception &e) {
        obj.reset();
        throw e;
    }
}

Widget::Widget(QWidget *parent) :
        QWidget(parent), ui(new Ui::Widget) {
    ui->setupUi(this);
}

void Widget::Construct() noexcept(false) {

}

Widget::~Widget() {
    delete ui;
}

