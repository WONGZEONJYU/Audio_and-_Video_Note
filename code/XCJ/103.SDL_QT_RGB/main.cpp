#include <QApplication>
#include "sdl_qt_rgb.hpp"
#include <SDL.h>

#undef main

int main(int argc, char *argv[]) {

    QApplication a(argc, argv);

    auto w{new sdl_qt_rgb()};
    w->show();

    return QApplication::exec();
}
