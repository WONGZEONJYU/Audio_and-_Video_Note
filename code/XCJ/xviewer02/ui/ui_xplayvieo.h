/********************************************************************************
** Form generated from reading UI file 'xplayvieo.ui'
**
** Created by: Qt User Interface Compiler version 6.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_XPLAYVIEO_H
#define UI_XPLAYVIEO_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_XPlayVieo
{
public:

    void setupUi(QWidget *XPlayVieo)
    {
        if (XPlayVieo->objectName().isEmpty())
            XPlayVieo->setObjectName("XPlayVieo");
        XPlayVieo->resize(835, 660);

        retranslateUi(XPlayVieo);

        QMetaObject::connectSlotsByName(XPlayVieo);
    } // setupUi

    void retranslateUi(QWidget *XPlayVieo)
    {
        XPlayVieo->setWindowTitle(QCoreApplication::translate("XPlayVieo", "XPlayVieo", nullptr));
    } // retranslateUi

};

namespace Ui {
    class XPlayVieo: public Ui_XPlayVieo {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_XPLAYVIEO_H
