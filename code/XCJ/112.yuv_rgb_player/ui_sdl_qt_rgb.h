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
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_sdl_qt_rgb
{
public:
    QLabel *video1;
    QLabel *video2;
    QLabel *view_fps2;
    QPushButton *open1;
    QComboBox *pix1;
    QSpinBox *width1;
    QSpinBox *height1;
    QSpinBox *set_fps1;
    QComboBox *pix2;
    QSpinBox *width2;
    QSpinBox *height2;
    QPushButton *open2;
    QSpinBox *set_fps2;
    QLabel *view_fps1;

    void setupUi(QWidget *sdl_qt_rgb)
    {
        if (sdl_qt_rgb->objectName().isEmpty())
            sdl_qt_rgb->setObjectName("sdl_qt_rgb");
        sdl_qt_rgb->resize(972, 517);
        video1 = new QLabel(sdl_qt_rgb);
        video1->setObjectName("video1");
        video1->setGeometry(QRect(10, 10, 471, 361));
        video1->setStyleSheet(QString::fromUtf8("background-color: rgb(4, 0, 255);"));
        video2 = new QLabel(sdl_qt_rgb);
        video2->setObjectName("video2");
        video2->setGeometry(QRect(510, 10, 451, 361));
        video2->setStyleSheet(QString::fromUtf8("background-color: rgb(61, 255, 0);"));
        view_fps2 = new QLabel(sdl_qt_rgb);
        view_fps2->setObjectName("view_fps2");
        view_fps2->setGeometry(QRect(520, 30, 61, 31));
        view_fps2->setStyleSheet(QString::fromUtf8("color: rgb(255, 0, 0);"));
        open1 = new QPushButton(sdl_qt_rgb);
        open1->setObjectName("open1");
        open1->setGeometry(QRect(130, 380, 111, 31));
        pix1 = new QComboBox(sdl_qt_rgb);
        pix1->addItem(QString());
        pix1->addItem(QString());
        pix1->addItem(QString());
        pix1->addItem(QString());
        pix1->addItem(QString());
        pix1->setObjectName("pix1");
        pix1->setGeometry(QRect(10, 380, 111, 32));
        width1 = new QSpinBox(sdl_qt_rgb);
        width1->setObjectName("width1");
        width1->setGeometry(QRect(10, 420, 91, 22));
        width1->setMinimum(1);
        width1->setMaximum(9999);
        width1->setValue(400);
        height1 = new QSpinBox(sdl_qt_rgb);
        height1->setObjectName("height1");
        height1->setGeometry(QRect(10, 450, 91, 22));
        height1->setMinimum(1);
        height1->setMaximum(9999);
        height1->setValue(300);
        set_fps1 = new QSpinBox(sdl_qt_rgb);
        set_fps1->setObjectName("set_fps1");
        set_fps1->setGeometry(QRect(10, 480, 91, 22));
        set_fps1->setMinimum(1);
        set_fps1->setMaximum(300);
        set_fps1->setValue(25);
        pix2 = new QComboBox(sdl_qt_rgb);
        pix2->addItem(QString());
        pix2->addItem(QString());
        pix2->addItem(QString());
        pix2->addItem(QString());
        pix2->addItem(QString());
        pix2->setObjectName("pix2");
        pix2->setGeometry(QRect(530, 380, 103, 32));
        width2 = new QSpinBox(sdl_qt_rgb);
        width2->setObjectName("width2");
        width2->setGeometry(QRect(530, 420, 91, 22));
        width2->setMinimum(1);
        width2->setMaximum(9999);
        width2->setValue(400);
        height2 = new QSpinBox(sdl_qt_rgb);
        height2->setObjectName("height2");
        height2->setGeometry(QRect(530, 450, 91, 22));
        height2->setMinimum(1);
        height2->setMaximum(9999);
        height2->setValue(300);
        open2 = new QPushButton(sdl_qt_rgb);
        open2->setObjectName("open2");
        open2->setGeometry(QRect(650, 380, 111, 31));
        set_fps2 = new QSpinBox(sdl_qt_rgb);
        set_fps2->setObjectName("set_fps2");
        set_fps2->setGeometry(QRect(530, 480, 91, 22));
        set_fps2->setMinimum(1);
        set_fps2->setMaximum(300);
        set_fps2->setValue(25);
        view_fps1 = new QLabel(sdl_qt_rgb);
        view_fps1->setObjectName("view_fps1");
        view_fps1->setGeometry(QRect(20, 20, 61, 31));
        view_fps1->setStyleSheet(QString::fromUtf8("color: rgb(255, 0, 0);"));

        retranslateUi(sdl_qt_rgb);
        QObject::connect(open1, SIGNAL(clicked()), sdl_qt_rgb, SLOT(Open1()));
        QObject::connect(open2, SIGNAL(clicked()), sdl_qt_rgb, SLOT(Open2()));

        QMetaObject::connectSlotsByName(sdl_qt_rgb);
    } // setupUi

    void retranslateUi(QWidget *sdl_qt_rgb)
    {
        sdl_qt_rgb->setWindowTitle(QCoreApplication::translate("sdl_qt_rgb", "sdl_qt_rgb", nullptr));
        video1->setText(QCoreApplication::translate("sdl_qt_rgb", "Video1", nullptr));
        video2->setText(QCoreApplication::translate("sdl_qt_rgb", "Video2", nullptr));
        view_fps2->setText(QCoreApplication::translate("sdl_qt_rgb", "FPS: 0", nullptr));
        open1->setText(QCoreApplication::translate("sdl_qt_rgb", "Open1", nullptr));
        pix1->setItemText(0, QCoreApplication::translate("sdl_qt_rgb", "YUV420P", nullptr));
        pix1->setItemText(1, QCoreApplication::translate("sdl_qt_rgb", "RGBA", nullptr));
        pix1->setItemText(2, QCoreApplication::translate("sdl_qt_rgb", "ARGB", nullptr));
        pix1->setItemText(3, QCoreApplication::translate("sdl_qt_rgb", "BGRA", nullptr));
        pix1->setItemText(4, QCoreApplication::translate("sdl_qt_rgb", "RGB24", nullptr));

        pix2->setItemText(0, QCoreApplication::translate("sdl_qt_rgb", "YUV420P", nullptr));
        pix2->setItemText(1, QCoreApplication::translate("sdl_qt_rgb", "RGBA", nullptr));
        pix2->setItemText(2, QCoreApplication::translate("sdl_qt_rgb", "ARGB", nullptr));
        pix2->setItemText(3, QCoreApplication::translate("sdl_qt_rgb", "BGRA", nullptr));
        pix2->setItemText(4, QCoreApplication::translate("sdl_qt_rgb", "RGB24", nullptr));

        open2->setText(QCoreApplication::translate("sdl_qt_rgb", "Open2", nullptr));
        view_fps1->setText(QCoreApplication::translate("sdl_qt_rgb", "FPS: 0", nullptr));
    } // retranslateUi

};

namespace Ui {
    class sdl_qt_rgb: public Ui_sdl_qt_rgb {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SDL_QT_RGB_H
