#ifndef XCAMERAWIDGET_HPP
#define XCAMERAWIDGET_HPP

#include <QWidget>

class XCameraWidget : public QWidget{

    Q_OBJECT
    //拖拽进入
    void dragEnterEvent(QDragEnterEvent *event) override;
    //拖拽松开
    void dropEvent(QDropEvent *event) override;

    void paintEvent(QPaintEvent *event) override;
public:
    explicit XCameraWidget(QWidget *parent = {});

};

#endif
