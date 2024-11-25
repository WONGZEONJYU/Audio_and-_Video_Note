#ifndef XPLAYVIEO_HPP
#define XPLAYVIEO_HPP

#include <QWidget>
#include "xvideo_widget.hpp"
#include <xplayer.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class XPlayVieo; }
QT_END_NAMESPACE

class XPlayVideo :
#ifdef MACOS
public XVideoWidget{
#else
public QWidget {
#endif

Q_OBJECT
    void timerEvent(QTimerEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
public:
    explicit XPlayVideo(QWidget *parent = nullptr);
    ~XPlayVideo() override ;
    bool Open(const QString &url);
    void Close();
private:
    QSharedPointer<Ui::XPlayVieo> m_ui_;
    XPlayer m_player_;
    int m_timer_id{-1};
};

#endif
