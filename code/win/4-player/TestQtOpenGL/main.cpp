#include <QApplication>
#include "widget.hpp"
#include <QSurfaceFormat>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
#if defined(__APPLE__) && defined(__MACH__)
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(4, 1); // 使用 OpenGL 4.1 核心配置
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);
#endif
    try {
        auto w{new_Widget()};
        w->show();
        return QApplication::exec();
    } catch (const std::exception &e) {
        qDebug() << e.what();
        return -1;
    }
}


