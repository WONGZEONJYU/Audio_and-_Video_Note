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
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>
#include <XVideoWidget.hpp>

QT_BEGIN_NAMESPACE

class Ui_XPlay2Widget
{
public:
    QGridLayout *gridLayout;
    XVideoWidget *VideoWidget;
    QPushButton *OpenFile;

    void setupUi(QWidget *XPlay2Widget)
    {
        if (XPlay2Widget->objectName().isEmpty())
            XPlay2Widget->setObjectName("XPlay2Widget");
        XPlay2Widget->resize(1280, 744);
        gridLayout = new QGridLayout(XPlay2Widget);
        gridLayout->setSpacing(0);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setContentsMargins(0, 0, 0, 0);
        VideoWidget = new XVideoWidget(XPlay2Widget);
        VideoWidget->setObjectName("VideoWidget");

        gridLayout->addWidget(VideoWidget, 0, 0, 1, 1);

        OpenFile = new QPushButton(XPlay2Widget);
        OpenFile->setObjectName("OpenFile");

        gridLayout->addWidget(OpenFile, 1, 0, 1, 1);


        retranslateUi(XPlay2Widget);
        QObject::connect(OpenFile, SIGNAL(clicked()), XPlay2Widget, SLOT(OpenFile()));

        QMetaObject::connectSlotsByName(XPlay2Widget);
    } // setupUi

    void retranslateUi(QWidget *XPlay2Widget)
    {
        XPlay2Widget->setWindowTitle(QCoreApplication::translate("XPlay2Widget", "XPlay2Widget", nullptr));
        OpenFile->setText(QCoreApplication::translate("XPlay2Widget", "Open Media File", nullptr));
    } // retranslateUi

};

namespace Ui {
    class XPlay2Widget: public Ui_XPlay2Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_XPLAY2WIDGET_H
