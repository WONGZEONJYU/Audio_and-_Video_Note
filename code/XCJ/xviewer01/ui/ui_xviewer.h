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
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_XViewer
{
public:
    QWidget *head;
    QWidget *logo;
    QPushButton *min;
    QPushButton *max;
    QPushButton *close;

    void setupUi(QWidget *XViewer)
    {
        if (XViewer->objectName().isEmpty())
            XViewer->setObjectName("XViewer");
        XViewer->resize(800, 600);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/img/xv.ico"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        XViewer->setWindowIcon(icon);
        XViewer->setStyleSheet(QString::fromUtf8("/*\351\241\266\351\203\250\346\240\267\345\274\217*/\n"
"#head{\n"
"	background-color: rgb(53, 53, 53);\n"
"}\n"
"#logo{\n"
"	background-image: url(:/img/logo_150_40.png);\n"
"}\n"
"#min{\n"
"	background-image: url(:/img/min.png);\n"
"}\n"
"#max{\n"
"	background-image: url(:/img/max.png);\n"
"}\n"
"#close{\n"
"	background-image: url(:/img/close.png);\n"
"}\n"
""));
        head = new QWidget(XViewer);
        head->setObjectName("head");
        head->setGeometry(QRect(0, 0, 800, 45));
        logo = new QWidget(head);
        logo->setObjectName("logo");
        logo->setGeometry(QRect(0, 3, 151, 40));
        min = new QPushButton(head);
        min->setObjectName("min");
        min->setGeometry(QRect(710, 10, 20, 20));
        min->setFlat(true);
        max = new QPushButton(head);
        max->setObjectName("max");
        max->setGeometry(QRect(740, 10, 20, 20));
        max->setFlat(true);
        close = new QPushButton(head);
        close->setObjectName("close");
        close->setGeometry(QRect(770, 10, 20, 20));
        close->setFlat(true);

        retranslateUi(XViewer);
        QObject::connect(close, &QPushButton::clicked, XViewer, qOverload<>(&QWidget::close));
        QObject::connect(max, &QPushButton::clicked, XViewer, qOverload<>(&QWidget::showMaximized));
        QObject::connect(min, &QPushButton::clicked, XViewer, qOverload<>(&QWidget::showMinimized));

        QMetaObject::connectSlotsByName(XViewer);
    } // setupUi

    void retranslateUi(QWidget *XViewer)
    {
        XViewer->setWindowTitle(QCoreApplication::translate("XViewer", "XViewer", nullptr));
        min->setText(QString());
        max->setText(QString());
        close->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class XViewer: public Ui_XViewer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_XVIEWER_H