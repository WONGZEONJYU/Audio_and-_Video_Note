/********************************************************************************
** Form generated from reading UI file 'xplayvideo.ui'
**
** Created by: Qt User Interface Compiler version 6.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_XPLAYVIDEO_H
#define UI_XPLAYVIDEO_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>
#include "xvideoshow.hpp"

QT_BEGIN_NAMESPACE

class Ui_XPlayVideo
{
public:
    XVideoShow *video;
    QSlider *speed;
    QLabel *speedtxt;
    QLabel *label;
    QSlider *pos;
    QPushButton *pause;

    void setupUi(QWidget *XPlayVideo)
    {
        if (XPlayVideo->objectName().isEmpty())
            XPlayVideo->setObjectName("XPlayVideo");
        XPlayVideo->resize(600, 400);
        XPlayVideo->setStyleSheet(QString::fromUtf8(""));
        video = new XVideoShow(XPlayVideo);
        video->setObjectName("video");
        video->setGeometry(QRect(0, 0, 600, 400));
        speed = new QSlider(XPlayVideo);
        speed->setObjectName("speed");
        speed->setGeometry(QRect(70, 5, 160, 31));
        speed->setMinimum(1);
        speed->setMaximum(20);
        speed->setPageStep(2);
        speed->setValue(10);
        speed->setOrientation(Qt::Orientation::Horizontal);
        speedtxt = new QLabel(XPlayVideo);
        speedtxt->setObjectName("speedtxt");
        speedtxt->setGeometry(QRect(240, 10, 50, 15));
        label = new QLabel(XPlayVideo);
        label->setObjectName("label");
        label->setGeometry(QRect(10, 10, 50, 15));
        pos = new QSlider(XPlayVideo);
        pos->setObjectName("pos");
        pos->setGeometry(QRect(80, 359, 511, 31));
        pos->setMinimum(1);
        pos->setMaximum(999);
        pos->setPageStep(100);
        pos->setOrientation(Qt::Orientation::Horizontal);
        pause = new QPushButton(XPlayVideo);
        pause->setObjectName("pause");
        pause->setGeometry(QRect(0, 330, 64, 64));
        pause->setStyleSheet(QString::fromUtf8("background-image: url(:/img/pause.png);\n"
"background-color: rgba(0, 0, 0,0);"));

        retranslateUi(XPlayVideo);
        QObject::connect(speed, SIGNAL(sliderReleased()), XPlayVideo, SLOT(SetSpeed()));
        QObject::connect(pause, SIGNAL(clicked()), XPlayVideo, SLOT(Pause()));
        QObject::connect(pos, SIGNAL(sliderReleased()), XPlayVideo, SLOT(PlayPos()));
        QObject::connect(pos, SIGNAL(sliderMoved(int)), XPlayVideo, SLOT(Move()));
        QObject::connect(pos, SIGNAL(sliderPressed()), XPlayVideo, SLOT(PlayPosPressed()));

        QMetaObject::connectSlotsByName(XPlayVideo);
    } // setupUi

    void retranslateUi(QWidget *XPlayVideo)
    {
        XPlayVideo->setWindowTitle(QCoreApplication::translate("XPlayVideo", "XPlayVieo", nullptr));
        speedtxt->setText(QCoreApplication::translate("XPlayVideo", "1.0", nullptr));
        label->setText(QCoreApplication::translate("XPlayVideo", "\346\222\255\346\224\276\351\200\237\345\272\246", nullptr));
        pause->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class XPlayVideo: public Ui_XPlayVideo {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_XPLAYVIDEO_H
