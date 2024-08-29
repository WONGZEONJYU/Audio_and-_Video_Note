//
// Created by wong on 2024/8/27.
//

#ifndef INC_103_SDL_QT_RGB_SDL_QT_RGB_HPP
#define INC_103_SDL_QT_RGB_SDL_QT_RGB_HPP

#include <QWidget>
#include <QVector>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui { class sdl_qt_rgb; }
QT_END_NAMESPACE

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
class XVideoView;

class sdl_qt_rgb : public QWidget {
Q_OBJECT

    void timerEvent(QTimerEvent *) override;
    void resizeEvent(QResizeEvent *) override;
public:
    explicit sdl_qt_rgb(QWidget * = nullptr);
    ~sdl_qt_rgb() override;

private:
    Ui::sdl_qt_rgb *ui;
    XVideoView *m_view{};
//    SDL_Window *m_screen{};
//    SDL_Renderer *m_renderer{};
//    SDL_Texture *m_texture{};
    int m_sdl_w{},m_sdl_h{},m_pix_size{2};
    QVector<uint8_t> m_yuv_datum;
    QFile m_yuv_file;
};

#endif

