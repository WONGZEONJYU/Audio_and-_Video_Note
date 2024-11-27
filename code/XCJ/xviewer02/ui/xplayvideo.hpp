#ifndef XPLAYVIEO_HPP
#define XPLAYVIEO_HPP

#include <QDialog>
#include "xvideo_widget.hpp"
#include <xplayer.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class XPlayVideo; }
QT_END_NAMESPACE

class XPlayVideo final:
#ifdef MACOS
public XVideoWidget {
#else
public QDialog {
#endif
Q_OBJECT
    void timerEvent(QTimerEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
public:
    explicit XPlayVideo(QWidget * = nullptr);
    ~XPlayVideo() override ;
    bool Open(const QString &url);
    void Close();

private slots:
     void SetSpeed();

#ifdef MACOS
    int exec();
#endif
private:
    QSharedPointer<Ui::XPlayVideo> m_ui_;
    XPlayer m_player_;
};

#endif
