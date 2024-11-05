#ifndef XVIEWER01_XVIEWER_HPP
#define XVIEWER01_XVIEWER_HPP

#include <QWidget>
#include <QSharedPointer>
#include <QMenu>
#include <QVector>
#include <QDate>

QT_BEGIN_NAMESPACE
namespace Ui { class XViewer; }
QT_END_NAMESPACE

class XViewer;
using XViewer_sp = QSharedPointer<XViewer>;
class XCameraWidget;
class XCameraRecord;

class XViewer final: public QWidget {
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

    /**
     * 预览窗口
     * @param count
     */
    void View(const int& count);

    /**
     * 刷新左侧相机列表
     */
    void RefreshCams() const;

    //编辑摄像机
    void SetCam(const int& index);

    //定时器渲染视频 回调函数
    void timerEvent(QTimerEvent *) override;
private slots:
    //大窗口,正常窗口
    void MaxWindow();
    void NormalWindow();
    void View1();
    void View4();
    void View9();
    void View16();
    void AddCam();
    void SetCam();
    void DelCam();

    void StartRecord();
    void StopRecord();

    void Preview() const; //预览界面
    void Playback() const; //回放界面

    void SelectCamera(const QModelIndex &index);
    void SelectDate(QDate date);
    void PlayVideo(const QModelIndex &index);

public:
    ~XViewer() override {
        Destroy();
    }

private:
    QVector<QSharedPointer<XCameraWidget>> m_cam_wins_;
    QVector<QSharedPointer<XCameraRecord>> m_cam_records_;
    QSharedPointer<Ui::XViewer> m_ui_{};
    QMenu m_left_menu_;
    bool m_is_mouse_pressed_{};
    QPointF m_mouse_pos_{};
public:
    static XViewer_sp create();
};

#endif
