#include <QApplication>
#include "XPlay2Widget.hpp"
//#include "XSonic.hpp"

int main(int argc, char *argv[]) {

#if 1
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
#else
    static int16_t src[1024]{},dst[2048]{};
//    XSonic sonic(44100,2);
//    sonic.sonicSetSpeed(1.5);
//    sonic.sonicWriteShortToStream(src,std::size(src));
//    qDebug() << sonic.sonicReadShortFromStream(dst,2048);
//    XSonic sonic;
//    qDebug() << sonic.sonicChangeShortSpeed(src,1024,1.5,1.0,1.0,1.0,0,44100,2);
//    return 0;
#endif
}

//rtmp://liteavapp.qcloud.com/live/liteavdemoplayerstreamid
