#ifndef XVIEWER01_XVIEWER_HPP
#define XVIEWER01_XVIEWER_HPP

#include <QWidget>
#include <QSharedPointer>
#include <QMenu>

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

    /***********鼠标移动事件,用于移动窗口***********/
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    /***********鼠标移动事件,用于移动窗口***********/

    //窗口大小发生变化
    void resizeEvent(QResizeEvent *event) override;

private slots:
    //大窗口,正常窗口
    void MaxWindow();
    void NormalWindow();

public:
    ~XViewer() override;

private:
    QSharedPointer<Ui::XViewer> m_ui_{};
    QMenu m_menu_;
    bool m_is_mouse_pressed_{};
    QPointF m_mouse_pos_{};

public:
    static XViewer_sp create();
};

#endif
