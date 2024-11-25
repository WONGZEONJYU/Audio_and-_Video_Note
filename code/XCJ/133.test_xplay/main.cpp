#if MACOS
#include <QApplication>
#include "xvideo_qtopengl_view.hpp"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    XVideoWidget w;
    return QApplication::exec();
}

#else
#include <xplayer.hpp>

int main(int argc, char *argv[]) {

    XPlayer xplayer;
    xplayer.Open("v1080.mp4");
    xplayer.Start();
    while (true) {
        xplayer.Update();
        if(xplayer.win_is_exit()){
            break;
        }
        XHelper::MSleep(10);
    }
    return 0;
}

#endif
