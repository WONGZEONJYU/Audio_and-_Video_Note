#ifndef XCAMERAWIDGET_HPP
#define XCAMERAWIDGET_HPP

#include <QWidget>
#include <QSharedPointer>

class XDecodeTask;
class XDemuxTask;
class XVideoView;

class XCameraWidget : public QWidget{

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
    void Draw() const;
private:
    QSharedPointer<XDecodeTask> m_decode_;
    QSharedPointer<XDemuxTask> m_demux_;
    QSharedPointer<XVideoView> m_view_;
};

#endif
