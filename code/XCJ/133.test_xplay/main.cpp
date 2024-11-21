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
#include <xplay.hpp>

int main(int argc, char *argv[]) {

    XPlay xplay;
    xplay.Open("10-8.mp4");
    xplay.Start();

    while (true) {
        xplay.Update();
        XHelper::MSleep(10);
    }

    getchar();
    return 0;
}

#endif
