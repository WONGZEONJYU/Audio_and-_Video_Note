//
// Created by wong on 2024/6/20.
//

#ifndef PLAYER_MAINWINDOW_HPP
#define PLAYER_MAINWINDOW_HPP

#include <QMainWindow>
#include <QPointer>
#include "MessageAbstract.hpp"
#include "ff_ffplay_def.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class IjkMediaPlayer;

class MainWindow final: public QMainWindow,
        public MessageAbstract<IjkMediaPlayer*>{
Q_OBJECT

    void construct() noexcept(false);
    explicit MainWindow(QWidget *parent = nullptr);
    friend class QSharedPointer<MainWindow> new_MainWindow() noexcept(false);
    void OnPlayOrPause();
    void OnStop();

    void msg_loop(Args_type &&) override;
    void closeEvent(QCloseEvent *event) override;
    bool event(QEvent *) override;
    int OutputVideo(const Frame &);
public:
    ~MainWindow() override;
private:
    Ui::MainWindow *ui;
    std::shared_ptr<IjkMediaPlayer> m_IjkMediaPlayer;
};

using MainWindow_sptr = QSharedPointer<MainWindow>;
MainWindow_sptr new_MainWindow() noexcept(false);

#endif
