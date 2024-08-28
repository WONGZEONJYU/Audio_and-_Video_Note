#include <QApplication>
#include "sdl_qt_rgb.hpp"

/**
 * 本实例代码可用于参考如何设计SDL结合QT显示图片,把SDL窗口嵌入到QT窗口中
 */

int main(int argc, char *argv[]) {

    QApplication a(argc, argv);
    auto w{new sdl_qt_rgb()};
    w->show();
    const auto ret{QApplication::exec()};
    delete w;
    return ret;
}
