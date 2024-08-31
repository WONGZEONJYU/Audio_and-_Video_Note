//
// Created by wong on 2024/8/27.
//

#ifndef INC_103_SDL_QT_RGB_SDL_QT_RGB_HPP
#define INC_103_SDL_QT_RGB_SDL_QT_RGB_HPP

#include <QWidget>
#include <QVector>
#include <QFile>
#include <thread>

QT_BEGIN_NAMESPACE
namespace Ui { class sdl_qt_rgb; }
QT_END_NAMESPACE

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
class XVideoView;
class XAVFrame;
class QLabel;
class QSpinBox;

using XAVFrame_sptr = typename std::shared_ptr<XAVFrame>;

class sdl_qt_rgb : public QWidget {
Q_OBJECT

    void timerEvent(QTimerEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    //视频刷新线程入口
    void Main();
public:
    explicit sdl_qt_rgb(QWidget * = nullptr);
    ~sdl_qt_rgb() override;

signals:
    void ViewS();

private slots:
    void View();

private:
    Ui::sdl_qt_rgb *ui;
    XVideoView *m_view{};
    QLabel *m_view_fps{};
    QSpinBox *m_SpinBox{};
    int m_sdl_w{},m_sdl_h{},
    m_pix_size{2},m_fps{25};
    XAVFrame_sptr m_frame;
    QFile m_yuv_file;
    std::thread m_th;
    std::atomic_bool m_th_is_exit{};
};

#endif

