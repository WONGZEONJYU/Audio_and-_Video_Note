//
// Created by Administrator on 2024/7/27.
//

// You may need to build the project (run Qt uic code generator) to get "ui_XPlay2Widget.h" resolved

#include "XPlay2Widget.hpp"
#include "ui_XPlay2Widget.h"
#include "XHelper.hpp"
#include <QFileDialog>
#include <XDemuxThread.hpp>

static XDemuxThread *dt;

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
    dt = new XDemuxThread();
    dt->Start();
}

void XPlay2Widget::DeConstruct() noexcept {
    delete dt;
    dt = nullptr;
}

void XPlay2Widget::OpenFile() {
    auto name {QFileDialog::getOpenFileName(this,QString::fromLocal8Bit("Select Media Files"))};
    if (name.isEmpty()){
        return;
    }
    setWindowTitle(name);

    try {
        dt->Open(name.toLocal8Bit(),m_ui->VideoWidget);
    } catch (const std::exception &e) {
        qDebug() << e.what();
        throw ;
    }
}
