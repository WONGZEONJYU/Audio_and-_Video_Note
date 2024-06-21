/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>
#include <ctrlbar.hpp>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QWidget *showCtrlBarBgWidget;
    CtrlBar *CtrlBarWidget;
    QWidget *ShowWidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;
    QDockWidget *PlaylistWidget;
    QWidget *PlaylistContents;
    QDockWidget *TitleWidget_3;
    QWidget *TitleContents_2;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 500);
        MainWindow->setAnimated(true);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        showCtrlBarBgWidget = new QWidget(centralwidget);
        showCtrlBarBgWidget->setObjectName("showCtrlBarBgWidget");
        showCtrlBarBgWidget->setGeometry(QRect(0, 0, 737, 397));
        CtrlBarWidget = new CtrlBar(showCtrlBarBgWidget);
        CtrlBarWidget->setObjectName("CtrlBarWidget");
        CtrlBarWidget->setGeometry(QRect(0, 337, 737, 60));
        CtrlBarWidget->setMinimumSize(QSize(0, 60));
        CtrlBarWidget->setMaximumSize(QSize(16777215, 60));
        ShowWidget = new QWidget(showCtrlBarBgWidget);
        ShowWidget->setObjectName("ShowWidget");
        ShowWidget->setGeometry(QRect(0, 0, 737, 337));
        ShowWidget->setMinimumSize(QSize(100, 100));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 37));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);
        PlaylistWidget = new QDockWidget(MainWindow);
        PlaylistWidget->setObjectName("PlaylistWidget");
        PlaylistWidget->setMinimumSize(QSize(62, 38));
        PlaylistContents = new QWidget();
        PlaylistContents->setObjectName("PlaylistContents");
        PlaylistWidget->setWidget(PlaylistContents);
        MainWindow->addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, PlaylistWidget);
        TitleWidget_3 = new QDockWidget(MainWindow);
        TitleWidget_3->setObjectName("TitleWidget_3");
        TitleWidget_3->setMinimumSize(QSize(62, 38));
        TitleWidget_3->setFloating(true);
        TitleContents_2 = new QWidget();
        TitleContents_2->setObjectName("TitleContents_2");
        TitleWidget_3->setWidget(TitleContents_2);
        MainWindow->addDockWidget(Qt::DockWidgetArea::TopDockWidgetArea, TitleWidget_3);

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
