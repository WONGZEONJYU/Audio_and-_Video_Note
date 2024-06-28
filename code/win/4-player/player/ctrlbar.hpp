//
// Created by Administrator on 2024/6/21.
//

#ifndef PLAYER_CTRLBAR_HPP
#define PLAYER_CTRLBAR_HPP

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class CtrlBar; }
QT_END_NAMESPACE

class CtrlBar : public QWidget {
Q_OBJECT

public:
    explicit CtrlBar(QWidget *parent = nullptr);
    ~CtrlBar() override;

private:
    Ui::CtrlBar *ui;

signals:
    void SigPlayOrPause();

};

#endif //PLAYER_CTRLBAR_HPP
