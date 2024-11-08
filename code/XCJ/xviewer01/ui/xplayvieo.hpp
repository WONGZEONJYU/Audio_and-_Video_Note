#ifndef XPLAYVIEO_HPP
#define XPLAYVIEO_HPP

#include <QWidget>
#include <xhelper.hpp>
#include <xdemuxtask.hpp>
#include <xdecodetask.hpp>
#include "xvideo_widget.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class XPlayVieo; }
QT_END_NAMESPACE

class XPlayVideo :
#ifdef MACOS
public XVideoWidget
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
    XDemuxTask m_demux_task_;
    XDecodeTask m_decode_task_;
#ifndef MACOS
    XVideoView_sp m_view_;
#endif
};

#endif
