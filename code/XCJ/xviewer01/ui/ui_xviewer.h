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
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_XViewer
{
public:
    QWidget *head;
    QWidget *logo;
    QWidget *head_button;
    QPushButton *min;
    QPushButton *max;
    QPushButton *close;
    QPushButton *normal;
    QWidget *body;
    QWidget *left;
    QListWidget *cam_list;
    QPushButton *add_cam;
    QPushButton *set_cam;
    QPushButton *del_cam;
    QWidget *cams;

    void setupUi(QWidget *XViewer)
    {
        if (XViewer->objectName().isEmpty())
            XViewer->setObjectName("XViewer");
        XViewer->resize(800, 600);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/img/xv.ico"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        XViewer->setWindowIcon(icon);
        XViewer->setStyleSheet(QString::fromUtf8("/*\346\214\211\351\222\256\346\240\267\345\274\217*/\n"
"\n"
"QPushButton:hover \n"
"{\n"
"	background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(0, 102, 184, 255), stop:0.495 rgba(39, 39, 39, 255), stop:0.505 rgba(39,39, 39, 255), stop:1 rgba(0, 102, 184, 255));\n"
"	border: none;\n"
"	border-radius:5px;\n"
"	color: rgb(255, 255, 255);\n"
"	font: 75 12pt \"\351\273\221\344\275\223\";\n"
" }\n"
"\n"
"QPushButton:!hover\n"
"{\n"
"	background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(0, 50, 150, 255), stop:0.495 rgba(0, 102, 184, 255), stop:0.505 rgba(0, 102, 184, 255), stop:1 rgba(0, 50, 150, 255));\n"
"	border: none;\n"
"	border-radius:5px;\n"
"	color: rgb(255, 255, 255);\n"
"	font: 75 12pt \"\351\273\221\344\275\223\";\n"
" }\n"
"\n"
"\n"
"/*\351\241\266\351\203\250\346\240\267\345\274\217*/\n"
"#head{\n"
"	background-color: rgb(53, 53, 53);\n"
"}\n"
"#logo{\n"
"	background-image: url(:/img/logo_150_40.png);\n"
"}\n"
"#min{\n"
"	background-image: url(:/"
                        "img/min.png);\n"
"}\n"
"#max{\n"
"	background-image: url(:/img/max.png);\n"
"}\n"
"#normal{\n"
"	\n"
"	background-image: url(:/img/normal.png);\n"
"}\n"
"#close{\n"
"	background-image: url(:/img/close.png);\n"
"}\n"
"\n"
"/*Body*/\n"
"#body{\n"
"	background-color: #212121;\n"
"}\n"
"#left{\n"
"	background-color: #252525;\n"
"}\n"
"#cams{\n"
"	background-color: #1e1e1e;\n"
"}\n"
""));
        head = new QWidget(XViewer);
        head->setObjectName("head");
        head->setGeometry(QRect(0, 0, 800, 50));
        head->setMaximumSize(QSize(16777215, 50));
        logo = new QWidget(head);
        logo->setObjectName("logo");
        logo->setGeometry(QRect(0, 3, 151, 40));
        head_button = new QWidget(head);
        head_button->setObjectName("head_button");
        head_button->setGeometry(QRect(680, 10, 120, 21));
        min = new QPushButton(head_button);
        min->setObjectName("min");
        min->setGeometry(QRect(10, 0, 20, 20));
        min->setFlat(true);
        max = new QPushButton(head_button);
        max->setObjectName("max");
        max->setGeometry(QRect(50, 0, 20, 20));
        max->setFlat(true);
        close = new QPushButton(head_button);
        close->setObjectName("close");
        close->setGeometry(QRect(90, 0, 20, 20));
        close->setFlat(true);
        normal = new QPushButton(head_button);
        normal->setObjectName("normal");
        normal->setGeometry(QRect(50, 0, 20, 20));
        normal->setFlat(true);
        min->raise();
        close->raise();
        normal->raise();
        max->raise();
        body = new QWidget(XViewer);
        body->setObjectName("body");
        body->setGeometry(QRect(10, 60, 781, 531));
        left = new QWidget(body);
        left->setObjectName("left");
        left->setGeometry(QRect(0, 0, 200, 521));
        left->setMaximumSize(QSize(200, 16777215));
        cam_list = new QListWidget(left);
        cam_list->setObjectName("cam_list");
        cam_list->setGeometry(QRect(0, 50, 200, 941));
        cam_list->setIconSize(QSize(50, 50));
        add_cam = new QPushButton(left);
        add_cam->setObjectName("add_cam");
        add_cam->setGeometry(QRect(5, 5, 61, 41));
        set_cam = new QPushButton(left);
        set_cam->setObjectName("set_cam");
        set_cam->setGeometry(QRect(70, 5, 61, 41));
        del_cam = new QPushButton(left);
        del_cam->setObjectName("del_cam");
        del_cam->setGeometry(QRect(135, 5, 61, 41));
        cams = new QWidget(body);
        cams->setObjectName("cams");
        cams->setGeometry(QRect(210, 0, 561, 521));

        retranslateUi(XViewer);
        QObject::connect(close, &QPushButton::clicked, XViewer, qOverload<>(&QWidget::close));
        QObject::connect(min, &QPushButton::clicked, XViewer, qOverload<>(&QWidget::showMinimized));
        QObject::connect(max, SIGNAL(clicked()), XViewer, SLOT(MaxWindow()));
        QObject::connect(normal, SIGNAL(clicked()), XViewer, SLOT(NormalWindow()));
        QObject::connect(add_cam, SIGNAL(clicked()), XViewer, SLOT(AddCam()));
        QObject::connect(set_cam, SIGNAL(clicked()), XViewer, SLOT(SetCam()));
        QObject::connect(del_cam, SIGNAL(clicked()), XViewer, SLOT(DelCam()));

        QMetaObject::connectSlotsByName(XViewer);
    } // setupUi

    void retranslateUi(QWidget *XViewer)
    {
        XViewer->setWindowTitle(QCoreApplication::translate("XViewer", "XViewer", nullptr));
        min->setText(QString());
        max->setText(QString());
        close->setText(QString());
        normal->setText(QString());
        add_cam->setText(QCoreApplication::translate("XViewer", "\346\226\260\345\242\236", nullptr));
        set_cam->setText(QCoreApplication::translate("XViewer", "\344\277\256\346\224\271", nullptr));
        del_cam->setText(QCoreApplication::translate("XViewer", "\345\210\240\351\231\244", nullptr));
    } // retranslateUi

};

namespace Ui {
    class XViewer: public Ui_XViewer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_XVIEWER_H
