#include <QApplication>
#include <QSurfaceFormat>
#include "XPlay2Widget.hpp"
#include "XDemux.hpp"
#include "XAVPacket.hpp"
#include "XAVCodecParameters.hpp"
#include "XDecode.hpp"

int main(int argc, char *argv[]) {
    //QApplication a(argc, argv);
    try {
        XDemux x;
        x.Open("2_audio.mp4");
        x.Open("2_audio.mp4");
        auto c {x.copy_ALLCodec_Parameters()};
//        for (auto &[n,item] : *c) {
//            qDebug() << n;
//        }

        //qDebug() << x.Seek(1.0);
//        while (true){
//            auto f{x.Read()};
//            if (f){
//                qDebug() << f->pos;
//            } else{
//                break;
//            }
//        }

        XDecode d;
        d.Open(c->at(2));

        return 0;
        //XPlay2Widget::Handle()->show();
        //return QApplication::exec();
    } catch (const std::exception &e) {
        qDebug() << e.what();
        return -1;
    }
}
