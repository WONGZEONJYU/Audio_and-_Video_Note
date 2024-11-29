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
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_XPlayVideo
{
public:
    QWidget *widget;
    QSlider *speed;
    QLabel *speedtxt;
    QLabel *label;
    QSlider *pos;

    void setupUi(QWidget *XPlayVideo)
    {
        if (XPlayVideo->objectName().isEmpty())
            XPlayVideo->setObjectName("XPlayVideo");
        XPlayVideo->resize(600, 400);
        widget = new QWidget(XPlayVideo);
        widget->setObjectName("widget");
        widget->setGeometry(QRect(0, 0, 600, 400));
        speed = new QSlider(XPlayVideo);
        speed->setObjectName("speed");
        speed->setGeometry(QRect(70, 10, 160, 16));
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
        pos->setGeometry(QRect(10, 380, 581, 16));
        pos->setMinimum(1);
        pos->setMaximum(999);
        pos->setPageStep(100);
        pos->setOrientation(Qt::Orientation::Horizontal);

        retranslateUi(XPlayVideo);
        QObject::connect(speed, SIGNAL(sliderReleased()), XPlayVideo, SLOT(SetSpeed()));

        QMetaObject::connectSlotsByName(XPlayVideo);
    } // setupUi

    void retranslateUi(QWidget *XPlayVideo)
    {
        XPlayVideo->setWindowTitle(QCoreApplication::translate("XPlayVideo", "XPlayVieo", nullptr));
        speedtxt->setText(QCoreApplication::translate("XPlayVideo", "1.0", nullptr));
        label->setText(QCoreApplication::translate("XPlayVideo", "\346\222\255\346\224\276\351\200\237\345\272\246", nullptr));
    } // retranslateUi

};

namespace Ui {
    class XPlayVideo: public Ui_XPlayVideo {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_XPLAYVIDEO_H
