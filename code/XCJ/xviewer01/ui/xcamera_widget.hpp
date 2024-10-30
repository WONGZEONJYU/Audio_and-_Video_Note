#ifndef XCAMERAWIDGET_HPP
#define XCAMERAWIDGET_HPP

#include <QWidget>
#include <QSharedPointer>
#include "XVideoWidget.hpp"

class XDecodeTask;
class XDemuxTask;
class XVideoView;

class XCameraWidget final:
#if 1
public XVideoWidget {
#else
public QWidget
#endif
    Q_OBJECT
    //拖拽进入
    void dragEnterEvent(QDragEnterEvent *event) override;
    //拖拽松开
    void dropEvent(QDropEvent *event) override;

    void paintEvent(QPaintEvent *event) override;
    //打开RTSP 开始解封封装,解码
    bool Open(const QString &url);

public:
    explicit XCameraWidget(QWidget *parent = {});

    ~XCameraWidget() override = default;
    //渲染视频
    void Draw() ;
private:
    QSharedPointer<XDecodeTask> m_decode_;
    QSharedPointer<XDemuxTask> m_demux_;
    QSharedPointer<XVideoView> m_view_;
    std::atomic_bool m_is_setStyle{};
};

#endif
