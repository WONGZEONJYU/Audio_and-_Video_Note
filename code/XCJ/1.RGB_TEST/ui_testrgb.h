/********************************************************************************
** Form generated from reading UI file 'testrgb.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TESTRGB_H
#define UI_TESTRGB_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TestRGB
{
public:

    void setupUi(QWidget *TestRGB)
    {
        if (TestRGB->objectName().isEmpty())
            TestRGB->setObjectName("TestRGB");
        TestRGB->resize(800, 600);

        retranslateUi(TestRGB);

        QMetaObject::connectSlotsByName(TestRGB);
    } // setupUi

    void retranslateUi(QWidget *TestRGB)
    {
        TestRGB->setWindowTitle(QCoreApplication::translate("TestRGB", "TestRGB", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TestRGB: public Ui_TestRGB {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TESTRGB_H
