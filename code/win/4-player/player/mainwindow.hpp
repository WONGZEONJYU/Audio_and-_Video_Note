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
    static QSharedPointer<MainWindow> create() noexcept(false);
public:

    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;
private:
    Ui::MainWindow *ui;
    friend class QSharedPointer<MainWindow> new_MainWindow() noexcept(false);
};

using MainWindow_sp_type = QSharedPointer<MainWindow>;

MainWindow_sp_type new_MainWindow() noexcept(false);

#endif //PLAYER_MAINWINDOW_HPP
