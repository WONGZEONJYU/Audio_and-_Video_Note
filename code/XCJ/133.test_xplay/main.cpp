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
    xplayer.Open("v1080.mp4");
    xplayer.Start();

    while (true) {
        xplayer.Update();
        XHelper::MSleep(10);
    }

    getchar();
    return 0;
}

#endif
