#if 0
#include <QApplication>
#include <QPushButton>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
     QPushButton button("Hello world!", nullptr);
     button.resize(200, 100);
     button.show();

    return QApplication::exec();
}
#else
#include <xplayer.hpp>

int main(int argc, char *argv[]) {

    XPlayer xplayer;
#if MACOS
    xplayer.Open("/Volumes/500G/v1080.mp4");
#else
    xplayer.Open("v1080.mp4");
#endif
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
