#ifndef XVIEWER01_XVIEWER_HPP
#define XVIEWER01_XVIEWER_HPP

#include <QWidget>
#include <QSharedPointer>
#include <QMenu>
#include <QVector>
#include <array>

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

    //窗口右键菜单
    void contextMenuEvent(QContextMenuEvent *event) override;

    void View(const int& count);

private slots:
    //大窗口,正常窗口
    void MaxWindow();
    void NormalWindow();
    void View1();
    void View4();
    void View9();
    void View16();

public:
    ~XViewer() override;

private:
    QVector<QSharedPointer<QWidget>> m_cam_wins_;
    QSharedPointer<Ui::XViewer> m_ui_{};
    QMenu m_left_menu_;
    bool m_is_mouse_pressed_{};
    QPointF m_mouse_pos_{};

public:
    static XViewer_sp create();
};

#endif
