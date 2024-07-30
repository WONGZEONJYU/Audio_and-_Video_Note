#include <QApplication>
#include <QSurfaceFormat>
#include "XPlay2Widget.hpp"
#include "XDemux.hpp"
#include "XAVPacket.hpp"
#include "XAVCodecParameters.hpp"

int main(int argc, char *argv[]) {
    //QApplication a(argc, argv);
    try {
        XDemux x;
        x.Open("2_audio.mp4");
        auto c {x.copy_ALLCodec_Parameters()};
        //XPlay2Widget::Handle()->show();
        //return QApplication::exec();
        return 0;
    } catch (const std::exception &e) {
        qDebug() << e.what();
        return -1;
    }
}
