/********************************************************************************
** Form generated from reading UI file 'xviewer.ui'
**
** Created by: Qt User Interface Compiler version 6.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_XVIEWER_H
#define UI_XVIEWER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_XViewer
{
public:

    void setupUi(QWidget *XViewer)
    {
        if (XViewer->objectName().isEmpty())
            XViewer->setObjectName("XViewer");
        XViewer->resize(400, 300);

        retranslateUi(XViewer);

        QMetaObject::connectSlotsByName(XViewer);
    } // setupUi

    void retranslateUi(QWidget *XViewer)
    {
        XViewer->setWindowTitle(QCoreApplication::translate("XViewer", "XViewer", nullptr));
    } // retranslateUi

};

namespace Ui {
    class XViewer: public Ui_XViewer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_XVIEWER_H
