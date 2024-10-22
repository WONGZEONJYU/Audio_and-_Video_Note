#include <QApplication>
#include "ui/xviewer.hpp"

int main(int argc,char *argv[]) {
    QApplication a(argc, argv);
    if (auto xviewer{XViewer::create()}){
        xviewer->show();
        return QApplication::exec();
    }
    return -1;
}
