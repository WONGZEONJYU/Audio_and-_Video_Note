#include <QApplication>
#include "sdl_qt_rgb.hpp"

int main(int argc, char *argv[]) {

    QApplication a(argc, argv);
    auto w{new sdl_qt_rgb()};
    w->show();
    const auto ret{QApplication::exec()};
    delete w;
    return ret;
}
