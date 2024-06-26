//
// Created by wong on 2024/6/20.
//

#ifndef PLAYER_MAINWINDOW_HPP
#define PLAYER_MAINWINDOW_HPP

#include <QMainWindow>
#include <QPointer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow final: public QMainWindow {
Q_OBJECT

    void construct() noexcept(false);
    explicit MainWindow(QWidget *parent = nullptr);
    friend class QPointer<MainWindow> new_MainWindow() noexcept(false);
public:
    ~MainWindow() override;

private:
    Ui::MainWindow *ui;
};
using MainWindow_sptr = QPointer<MainWindow>;
MainWindow_sptr new_MainWindow() noexcept(false);

#endif //PLAYER_MAINWINDOW_HPP
