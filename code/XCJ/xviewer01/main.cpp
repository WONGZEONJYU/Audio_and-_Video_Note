#include <QApplication>
#include "ui/xviewer.hpp"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    auto xviewer{XViewer::create()};
    if (xviewer){
        xviewer->show();
        return QApplication::exec();
    }
    return -1;
}
