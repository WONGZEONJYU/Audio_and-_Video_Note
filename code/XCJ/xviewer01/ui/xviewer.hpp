#ifndef XVIEWER01_XVIEWER_HPP
#define XVIEWER01_XVIEWER_HPP

#include <QWidget>
#include <QSharedPointer>

QT_BEGIN_NAMESPACE
namespace Ui { class XViewer; }
QT_END_NAMESPACE

class XViewer;
using XViewer_sp = QSharedPointer<XViewer>;

class XViewer : public QWidget {
Q_OBJECT
    explicit XViewer(QWidget *parent = nullptr);
    bool Construct();
    void Destroy();



    //鼠标移动事件,用于移动窗口
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
public:
    ~XViewer() override;

private:
    QSharedPointer<Ui::XViewer> m_ui_{};
    bool m_is_mouse_pressed_{};
    QPointF m_mouse_pos_{};
public:
    static XViewer_sp create();
};

#endif
