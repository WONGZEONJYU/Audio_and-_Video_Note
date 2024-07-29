#include <QApplication>
#include <QSurfaceFormat>
#include "XPlay2Widget.hpp"
#include "XDemux.hpp"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    try {

        XDemux de,d1;
        de.Open("2_audio.mp4");
        d1.Open("rtmp://live.hkstv.hk.lxdns.com/live/hks");

        XPlay2Widget::Handle()->show();
        return QApplication::exec();
    } catch (const std::exception &e) {
        qDebug() << e.what();
        return -1;
    }
}
