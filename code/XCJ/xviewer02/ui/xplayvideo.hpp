#ifndef XPLAYVIEO_HPP
#define XPLAYVIEO_HPP

#include <QDialog>
#include "xvideo_widget.hpp"
#include <xplayer.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class XPlayVideo; }
QT_END_NAMESPACE

class XPlayVideo final: public QDialog {

Q_OBJECT
    void timerEvent(QTimerEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
public:
    explicit XPlayVideo(QWidget * = {});
    ~XPlayVideo() override ;
    bool Open(const QString &url);
    void Close();

private slots:
     void SetSpeed();
     void Pause();
     void PlayPos();
    void PlayPosPressed();
     void Move();
private:
    QSharedPointer<Ui::XPlayVideo> m_ui_;
    XPlayer m_player_;
    std::atomic_bool m_is_move_{},m_is_press_{};

};

#endif
