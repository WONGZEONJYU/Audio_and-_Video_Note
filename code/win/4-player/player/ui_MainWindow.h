/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>
#include <ctrlbar.hpp>
#include <titlebar.hpp>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QWidget *showCtrlBarBgWidget;
    QGridLayout *gridLayout_2;
    QWidget *ShowWidget;
    CtrlBar *CtrlBarWidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;
    QDockWidget *PlaylistWidget;
    QWidget *PlaylistContents;
    QDockWidget *TitleWidget_3;
    TitleBar *TitleContents_2;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 500);
        MainWindow->setAnimated(true);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setSpacing(0);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setContentsMargins(0, 0, 0, 0);
        showCtrlBarBgWidget = new QWidget(centralwidget);
        showCtrlBarBgWidget->setObjectName("showCtrlBarBgWidget");
        gridLayout_2 = new QGridLayout(showCtrlBarBgWidget);
        gridLayout_2->setSpacing(0);
        gridLayout_2->setObjectName("gridLayout_2");
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        ShowWidget = new QWidget(showCtrlBarBgWidget);
        ShowWidget->setObjectName("ShowWidget");
        ShowWidget->setMinimumSize(QSize(100, 100));

        gridLayout_2->addWidget(ShowWidget, 0, 0, 1, 1);

        CtrlBarWidget = new CtrlBar(showCtrlBarBgWidget);
        CtrlBarWidget->setObjectName("CtrlBarWidget");
        CtrlBarWidget->setMinimumSize(QSize(0, 60));
        CtrlBarWidget->setMaximumSize(QSize(16777215, 60));

        gridLayout_2->addWidget(CtrlBarWidget, 1, 0, 1, 1);


        gridLayout->addWidget(showCtrlBarBgWidget, 0, 0, 1, 1);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 22));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);
        PlaylistWidget = new QDockWidget(MainWindow);
        PlaylistWidget->setObjectName("PlaylistWidget");
        PlaylistWidget->setMinimumSize(QSize(81, 39));
        PlaylistContents = new QWidget();
        PlaylistContents->setObjectName("PlaylistContents");
        PlaylistWidget->setWidget(PlaylistContents);
        MainWindow->addDockWidget(Qt::RightDockWidgetArea, PlaylistWidget);
        TitleWidget_3 = new QDockWidget(MainWindow);
        TitleWidget_3->setObjectName("TitleWidget_3");
        TitleWidget_3->setMinimumSize(QSize(81, 39));
        TitleWidget_3->setFloating(true);
        TitleContents_2 = new TitleBar();
        TitleContents_2->setObjectName("TitleContents_2");
        TitleWidget_3->setWidget(TitleContents_2);
        MainWindow->addDockWidget(Qt::TopDockWidgetArea, TitleWidget_3);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
