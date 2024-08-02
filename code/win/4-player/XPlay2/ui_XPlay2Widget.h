/********************************************************************************
** Form generated from reading UI file 'XPlay2Widget.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_XPLAY2WIDGET_H
#define UI_XPLAY2WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <XVideoWidget.hpp>

QT_BEGIN_NAMESPACE

class Ui_XPlay2Widget
{
public:
    XVideoWidget *VideoWidget;

    void setupUi(QWidget *XPlay2Widget)
    {
        if (XPlay2Widget->objectName().isEmpty())
            XPlay2Widget->setObjectName("XPlay2Widget");
        XPlay2Widget->resize(1920, 1080);
        VideoWidget = new XVideoWidget(XPlay2Widget);
        VideoWidget->setObjectName("VideoWidget");
        VideoWidget->setGeometry(QRect(0, 0, 1920, 1080));

        retranslateUi(XPlay2Widget);

        QMetaObject::connectSlotsByName(XPlay2Widget);
    } // setupUi

    void retranslateUi(QWidget *XPlay2Widget)
    {
        XPlay2Widget->setWindowTitle(QCoreApplication::translate("XPlay2Widget", "XPlay2Widget", nullptr));
    } // retranslateUi

};

namespace Ui {
    class XPlay2Widget: public Ui_XPlay2Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_XPLAY2WIDGET_H
