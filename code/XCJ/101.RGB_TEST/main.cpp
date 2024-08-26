#include <QApplication>
#include <QPushButton>
#include "testrgb.hpp"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    auto w{new TestRGB()};
    w->show();
    const auto r{QApplication::exec()};
    delete w;
    return r;
}
