//
// Created by wong on 2024/6/22.
//

#ifndef PLAYER_DISPLAYBOX_HPP
#define PLAYER_DISPLAYBOX_HPP

#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class DisplayBox; }
QT_END_NAMESPACE

class DisplayBox : public QWidget {
    Q_OBJECT

public:
    explicit DisplayBox(QWidget *parent = nullptr);

    ~DisplayBox() override;

private:
    Ui::DisplayBox *ui;
};


#endif //PLAYER_DISPLAYBOX_HPP
