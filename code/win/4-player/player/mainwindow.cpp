//
// Created by wong on 2024/6/20.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "mainwindow.hpp"
#include <memory>
#include "ui_MainWindow.h"
#include "IjkMediaPlayer.hpp"
#include "src/ff_ffmsg.h"

class FFMSG : public QEvent{
    int m_msg{};
public:
    explicit FFMSG(const int&msg):QEvent{QEvent::User},m_msg{msg}{}
    [[nodiscard]] auto msg() const {return m_msg;}
};

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->CtrlBarWidget,&CtrlBar::SigPlayOrPause,this,&MainWindow::OnPlayOrPause);
    connect(ui->CtrlBarWidget,&CtrlBar::SigStop,this,&MainWindow::OnStop);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::construct() noexcept(false) {

}

MainWindow_sptr new_MainWindow() noexcept(false)
{
    MainWindow_sptr obj;
    try {
        obj = MainWindow_sptr (new MainWindow);
        obj->construct();
        return obj;
    } catch (const std::bad_alloc &e) {
        obj.reset();
        throw std::runtime_error("new MainWindow failed\n");
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw e;
    }
}

void MainWindow::OnPlayOrPause() {

    if (m_IjkMediaPlayer){ //已经创建,则执行播放或暂停

    } else{ //m_IjkMediaPlayer还没创建,则创建
        try {
            m_IjkMediaPlayer = new_IjkMediaPlayer(*this);
            m_IjkMediaPlayer->Add_VideoRefreshCallback(
                    [this](auto && vp) {
                        return OutputVideo(std::forward<decltype(vp)>(vp));
                    });
            m_IjkMediaPlayer->set_data_source("2_audio.mp4");
            m_IjkMediaPlayer->prepare_async();

        } catch (const std::exception &e) {
            qDebug() << e.what();
            m_IjkMediaPlayer.reset();
            close();
        }
    }
}

void MainWindow::OnStop() {

    if (m_IjkMediaPlayer){
        m_IjkMediaPlayer->stop();
        m_IjkMediaPlayer.reset();
    }
}

void MainWindow::msg_loop(Args_type &&obj) {

    auto obj1{get<0>(obj)};
    while (true){
        AVMessage_Sptr msg;
        const auto ret{obj1->get_msg(msg, true)};
        if (ret < 0){
            return;
        }
        const auto what(msg->what());
        switch (what) {
            case FFP_MSG_FLUSH:
                qDebug() << __FUNCTION__ << "\tFFP_MSG_FLUSH";
                break;
            case FFP_MSG_PREPARED:
                m_IjkMediaPlayer->start();
                qDebug() << __FUNCTION__ << "\tFFP_MSG_PREPARED";
                break;
            case FFP_MSG_ERROR:
                qDebug() << __FUNCTION__ << "\tFFP_MSG_ERROR";
                qApp->postEvent(this,new FFMSG(FFP_MSG_ERROR));
                break;
            default:
                qDebug() << __FUNCTION__ << "\tother_msg:\t" << what;
                break;
        }
    }
}

int MainWindow::OutputVideo(const Frame &vp)
{

}

void MainWindow::closeEvent(QCloseEvent *event) {
    QWidget::closeEvent(event);
    OnStop();
}

bool MainWindow::event(QEvent *e) {

    if (e->type() == QEvent::User){
        auto msg{dynamic_cast<FFMSG*>(e)};
        if (msg->msg() == FFP_MSG_ERROR) {
            OnStop();
        }
    }

    return QMainWindow::event( e);
}
