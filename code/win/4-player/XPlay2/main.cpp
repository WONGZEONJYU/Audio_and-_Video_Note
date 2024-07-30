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
        for (auto &item : *c) {
            qDebug() << item.get();
        }
        //XPlay2Widget::Handle()->show();
        //return QApplication::exec();
        //qDebug() << x.Seek(1.0);
//        while (true){
//            auto f{x.Read()};
//            if (f){
//                qDebug() << f->pos;
//            } else{
//                break;
//            }
//        }
        x.Clear();
        x.Close();
        x.Seek(0.95);
        return 0;
    } catch (const std::exception &e) {
        qDebug() << e.what();
        return -1;
    }
}
