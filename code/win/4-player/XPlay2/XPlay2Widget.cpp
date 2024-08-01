//
// Created by Administrator on 2024/7/27.
//

// You may need to build the project (run Qt uic code generator) to get "ui_XPlay2Widget.h" resolved

#include "XPlay2Widget.hpp"
#include "ui_XPlay2Widget.h"
#include "XHelper.hpp"


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
    //show();
    m_ui->VideoWidget->Init(800,600);
}

void XPlay2Widget::DeConstruct() noexcept {

}
