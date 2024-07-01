//
// Created by wong on 2024/6/22.
//

// You may need to build the project (run Qt uic code generator) to get "ui_PlayListBox.h" resolved

#include "playlistbox.hpp"
#include "ui_playlistbox.h"


PlayListBox::PlayListBox(QWidget *parent) :
        QWidget(parent), ui(new Ui::PlayListBox) {
    ui->setupUi(this);
}

PlayListBox::~PlayListBox() {
    delete ui;
}
