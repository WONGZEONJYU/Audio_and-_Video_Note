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
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>
#include <ctrlbar.hpp>
#include <displaybox.hpp>
#include <playlistbox.hpp>
#include <titlebar.hpp>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QWidget *showCtrlBarBgWidget;
    QGridLayout *gridLayout_2;
    CtrlBar *CtrlBarWidget;
    DisplayBox *ShowWidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;
    QDockWidget *PlaylistWidget;
    PlayListBox *PlaylistContents;
    QDockWidget *TitleWidget_3;
    TitleBar *TitleContents_2;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1281, 909);
        MainWindow->setMinimumSize(QSize(1280, 909));
        MainWindow->setMaximumSize(QSize(16777215, 16777215));
        MainWindow->setAnimated(true);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        centralwidget->setMinimumSize(QSize(1080, 740));
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setSpacing(0);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setContentsMargins(0, 0, 0, 0);
        showCtrlBarBgWidget = new QWidget(centralwidget);
        showCtrlBarBgWidget->setObjectName("showCtrlBarBgWidget");
        showCtrlBarBgWidget->setMinimumSize(QSize(1080, 740));
        gridLayout_2 = new QGridLayout(showCtrlBarBgWidget);
        gridLayout_2->setSpacing(0);
        gridLayout_2->setObjectName("gridLayout_2");
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        CtrlBarWidget = new CtrlBar(showCtrlBarBgWidget);
        CtrlBarWidget->setObjectName("CtrlBarWidget");
        CtrlBarWidget->setMinimumSize(QSize(1080, 60));
        CtrlBarWidget->setMaximumSize(QSize(1080, 60));

        gridLayout_2->addWidget(CtrlBarWidget, 1, 0, 1, 1);

        ShowWidget = new DisplayBox(showCtrlBarBgWidget);
        ShowWidget->setObjectName("ShowWidget");
        ShowWidget->setMinimumSize(QSize(1080, 680));
        ShowWidget->setMaximumSize(QSize(16777215, 16777215));

        gridLayout_2->addWidget(ShowWidget, 0, 0, 1, 1);


        gridLayout->addWidget(showCtrlBarBgWidget, 0, 0, 1, 1);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setEnabled(true);
        menubar->setGeometry(QRect(0, 0, 1281, 37));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        statusbar->setEnabled(true);
        MainWindow->setStatusBar(statusbar);
        PlaylistWidget = new QDockWidget(MainWindow);
        PlaylistWidget->setObjectName("PlaylistWidget");
        PlaylistWidget->setMinimumSize(QSize(200, 762));
        PlaylistContents = new PlayListBox();
        PlaylistContents->setObjectName("PlaylistContents");
        PlaylistContents->setMinimumSize(QSize(200, 740));
        PlaylistWidget->setWidget(PlaylistContents);
        MainWindow->addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, PlaylistWidget);
        TitleWidget_3 = new QDockWidget(MainWindow);
        TitleWidget_3->setObjectName("TitleWidget_3");
        TitleWidget_3->setMinimumSize(QSize(1280, 82));
        TitleWidget_3->setFloating(false);
        TitleContents_2 = new TitleBar();
        TitleContents_2->setObjectName("TitleContents_2");
        TitleContents_2->setMinimumSize(QSize(1280, 60));
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
