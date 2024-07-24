#include <QApplication>
#include "widget.hpp"
#include <QSurfaceFormat>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    try {
        auto w{new_Widget()};
        w->show();
        return QApplication::exec();
    } catch (const std::exception &e) {
        qDebug() << e.what();
        return -1;
    }
}


