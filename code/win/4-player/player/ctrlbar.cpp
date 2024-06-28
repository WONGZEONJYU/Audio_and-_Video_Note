//
// Created by Administrator on 2024/6/21.
//

// You may need to build the project (run Qt uic code generator) to get "ui_CtrlBar.h" resolved

#include "ctrlbar.hpp"
#include "ui_ctrlbar.h"

CtrlBar::CtrlBar(QWidget *parent) :
        QWidget(parent), ui(new Ui::CtrlBar) {
    ui->setupUi(this);

    connect(ui->PlayOrPauseBtn,&QPushButton::clicked, [this](bool ){
        emit SigPlayOrPause();
    });
}

CtrlBar::~CtrlBar() {
    delete ui;
}
