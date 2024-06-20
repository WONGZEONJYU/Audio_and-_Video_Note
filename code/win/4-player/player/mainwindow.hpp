//
// Created by Administrator on 2024/6/20.
//

#ifndef PLAYER_MAINWINDOW_HPP
#define PLAYER_MAINWINDOW_HPP

#include <QMainWindow>
#include <QSharedPointer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow final : public QMainWindow {
Q_OBJECT

    void construct() noexcept(false);
public:
    using MainWindow_sp_type = QSharedPointer<MainWindow>;
    explicit MainWindow(QWidget *parent = nullptr);
    static MainWindow_sp_type create() noexcept(false);
    ~MainWindow() override;
private:
    Ui::MainWindow *ui;
};

using MainWindow_sp_type = MainWindow::MainWindow_sp_type;

#endif //PLAYER_MAINWINDOW_HPP
