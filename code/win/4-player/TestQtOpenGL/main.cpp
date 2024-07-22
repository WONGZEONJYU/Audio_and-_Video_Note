#include <QApplication>
#include "widget.hpp"
#include <QSurfaceFormat>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(4, 1); // 使用 OpenGL 3.3 核心配置
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    try {
        auto w{new_Widget()};
        w->show();
        return QApplication::exec();
    } catch (const std::exception &e) {
        qDebug() << e.what();
        return -1;
    }
}
