#include <QApplication>
#include "XPlay2Widget.hpp"

int main(int argc, char *argv[]) {

    QApplication a(argc, argv);
    XPlay2Widget_sptr w;
    try {
        w = XPlay2Widget::Handle();
        w->show();
        return QApplication::exec();
    } catch (const std::exception &e) {
        qDebug() << e.what();
        return -1;
    }
}
