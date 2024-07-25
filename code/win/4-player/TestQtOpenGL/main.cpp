#include <QApplication>
#include "widget.hpp"
#include <QSurfaceFormat>
#include <QOpenGLContext>

int main(int argc, char *argv[]) {

#if defined(__APPLE__) && defined(__MACH__)
    QSurfaceFormat format;
    format.setVersion(4, 1); // 或 3, 2，取决于你的 OpenGL 版本要求
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

#endif

    QApplication a(argc, argv);

    try {
        auto w{new_Widget()};
        w->show();
        return QApplication::exec();
    } catch (const std::exception &e) {
        qDebug() << e.what();
        return -1;
    }
}


