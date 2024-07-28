#include <QApplication>
#include <QSurfaceFormat>
#include "XPlay2Widget.hpp"
#include "XDemux.hpp"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    try {

        XDemux dmux,d1;
        dmux.Open("2_audio_track_5s.mp4");
        d1.Open("2_audio_track_5s.mp4");

        XPlay2Widget::Handle()->show();
        return QApplication::exec();
    } catch (const std::exception &e) {
        qDebug() << e.what();
        return -1;
    }
}
