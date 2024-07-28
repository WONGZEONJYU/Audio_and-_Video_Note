//
// Created by Administrator on 2024/7/27.
//

// You may need to build the project (run Qt uic code generator) to get "ui_XPlay2Widget.h" resolved

#include "XPlay2Widget.hpp"
#include "ui_XPlay2Widget.h"
#include <QMUtex>

XPlay2Widget_sptr XPlay2Widget::uni_win{};

XPlay2Widget_sptr XPlay2Widget::Handle() noexcept(false){

    static QMutex mux;
    QMutexLocker locker(&mux);
    try {
        if (!uni_win){
            uni_win.reset(new XPlay2Widget);
            uni_win->Construct();
        }
        return uni_win;
    } catch (const std::bad_alloc &e) {
        uni_win.reset();
        locker.unlock();
        throw std::runtime_error("new XPlay2Widget failed");
    } catch (const std::runtime_error &e) {
        uni_win.reset();
        locker.unlock();
        throw e;
    }
}

XPlay2Widget::XPlay2Widget(QWidget *parent) :
        QWidget(parent) {
}

XPlay2Widget::~XPlay2Widget() {
    DeConstruct();
}

void XPlay2Widget::Construct() noexcept(false) {

    try {
        ui = new Ui::XPlay2Widget;
        ui->setupUi(this);
    } catch (const std::bad_alloc &e) {
        throw std::runtime_error("new Ui::XPlay2Widget failed");
    }
}

void XPlay2Widget::DeConstruct() noexcept {
    delete ui;
}
