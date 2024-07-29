#include <QApplication>
#include <QSurfaceFormat>
#include "XPlay2Widget.hpp"
#include "XDemux.hpp"
#include "XAVPacket.hpp"

int main(int argc, char *argv[]) {
    //QApplication a(argc, argv);
    try {
        XDemux x;
        x.Open("2_audio.mp4");

        //XPlay2Widget::Handle()->show();
        //return QApplication::exec();
        return 0;
    } catch (const std::exception &e) {
        qDebug() << e.what();
        return -1;
    }
}
