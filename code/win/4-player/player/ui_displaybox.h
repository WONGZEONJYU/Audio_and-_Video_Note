/********************************************************************************
** Form generated from reading UI file 'displaybox.ui'
**
** Created by: Qt User Interface Compiler version 6.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DISPLAYBOX_H
#define UI_DISPLAYBOX_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DisplayBox
{
public:
    QGridLayout *gridLayout;
    QLabel *label;

    void setupUi(QWidget *DisplayBox)
    {
        if (DisplayBox->objectName().isEmpty())
            DisplayBox->setObjectName("DisplayBox");
        DisplayBox->resize(531, 394);
        gridLayout = new QGridLayout(DisplayBox);
        gridLayout->setObjectName("gridLayout");
        label = new QLabel(DisplayBox);
        label->setObjectName("label");
        label->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 0, 0);"));
        label->setAlignment(Qt::AlignmentFlag::AlignCenter);

        gridLayout->addWidget(label, 0, 0, 1, 1);


        retranslateUi(DisplayBox);

        QMetaObject::connectSlotsByName(DisplayBox);
    } // setupUi

    void retranslateUi(QWidget *DisplayBox)
    {
        DisplayBox->setWindowTitle(QCoreApplication::translate("DisplayBox", "DisplayBox", nullptr));
        label->setText(QCoreApplication::translate("DisplayBox", "TextLabel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DisplayBox: public Ui_DisplayBox {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DISPLAYBOX_H
