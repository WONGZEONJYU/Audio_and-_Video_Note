//
// Created by Administrator on 2024/6/22.
//

// You may need to build the project (run Qt uic code generator) to get "ui_TitleBar.h" resolved

#include "titlebar.hpp"
#include "ui_titlebar.h"


TitleBar::TitleBar(QWidget *parent) :
        QWidget(parent), ui(new Ui::TitleBar)
{
    ui->setupUi(this);
}

TitleBar::~TitleBar() {
    delete ui;
}
