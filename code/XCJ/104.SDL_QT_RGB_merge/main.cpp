#include <QApplication>
#include "sdl_qt_rgb.hpp"
/**
 * 本测试代码用于视频合并画面设计和截图设计
 */

int main(int argc, char *argv[]) {

    QApplication a(argc, argv);
    auto w{new sdl_qt_rgb()};
    w->show();
    const auto ret{QApplication::exec()};
    delete w;
    return ret;
}
