//
// Created by wong on 2024/8/27.
//

#ifndef INC_103_SDL_QT_RGB_SDL_QT_RGB_HPP
#define INC_103_SDL_QT_RGB_SDL_QT_RGB_HPP

#include <QWidget>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui { class sdl_qt_rgb; }
QT_END_NAMESPACE

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

class sdl_qt_rgb : public QWidget {
Q_OBJECT

    void timerEvent(QTimerEvent *) override;

public:
    explicit sdl_qt_rgb(QWidget *parent = nullptr);

    ~sdl_qt_rgb() override;

private:
    Ui::sdl_qt_rgb *ui;
    SDL_Window *m_screen{};
    SDL_Renderer *m_renderer{};
    SDL_Texture *m_texture{};
    int m_w{},m_h{};
    uint32_t m_count{255},m_i{},m_rgb[3]{255};

    QVector<uint8_t> m_rgb_datum;

};

#endif

