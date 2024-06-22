//
// Created by Administrator on 2024/6/22.
//

#ifndef PLAYER_TITLEBAR_HPP
#define PLAYER_TITLEBAR_HPP

#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class TitleBar; }
QT_END_NAMESPACE

class TitleBar : public QWidget {
Q_OBJECT

public:
    explicit TitleBar(QWidget *parent = nullptr);

    ~TitleBar() override;

private:
    Ui::TitleBar *ui;
};


#endif //PLAYER_TITLEBAR_HPP
