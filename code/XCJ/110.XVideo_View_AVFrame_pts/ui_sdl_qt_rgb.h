/********************************************************************************
** Form generated from reading UI file 'sdl_qt_rgb.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SDL_QT_RGB_H
#define UI_SDL_QT_RGB_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_sdl_qt_rgb
{
public:
    QLabel *view_fps;
    QWidget *widget;

    void setupUi(QWidget *sdl_qt_rgb)
    {
        if (sdl_qt_rgb->objectName().isEmpty())
            sdl_qt_rgb->setObjectName("sdl_qt_rgb");
        sdl_qt_rgb->resize(800, 600);
        view_fps = new QLabel(sdl_qt_rgb);
        view_fps->setObjectName("view_fps");
        view_fps->setGeometry(QRect(10, 0, 61, 31));
        widget = new QWidget(sdl_qt_rgb);
        widget->setObjectName("widget");
        widget->setGeometry(QRect(80, 60, 631, 471));

        retranslateUi(sdl_qt_rgb);

        QMetaObject::connectSlotsByName(sdl_qt_rgb);
    } // setupUi

    void retranslateUi(QWidget *sdl_qt_rgb)
    {
        sdl_qt_rgb->setWindowTitle(QCoreApplication::translate("sdl_qt_rgb", "sdl_qt_rgb", nullptr));
        view_fps->setText(QCoreApplication::translate("sdl_qt_rgb", "TextLabel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class sdl_qt_rgb: public Ui_sdl_qt_rgb {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SDL_QT_RGB_H
