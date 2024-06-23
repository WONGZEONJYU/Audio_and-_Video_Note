//
// Created by wong on 2024/6/22.
//

// You may need to build the project (run Qt uic code generator) to get "ui_DisplayBox.h" resolved

#include "displaybox.hpp"
#include "ui_DisplayBox.h"


DisplayBox::DisplayBox(QWidget *parent) :
        QWidget(parent), ui(new Ui::DisplayBox) {
    ui->setupUi(this);
}

DisplayBox::~DisplayBox() {
    delete ui;
}
