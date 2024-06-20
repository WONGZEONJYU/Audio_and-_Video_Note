#include <QApplication>
#include "mainwindow.hpp"

int main(int argc, char *argv[]) {

    QApplication a(argc, argv);

    try {
        auto win{new_MainWindow()};
        win->show();
    } catch (const std::exception &e) {
        qDebug() << e.what();
        return -1;
    }

    return QApplication::exec();
}
