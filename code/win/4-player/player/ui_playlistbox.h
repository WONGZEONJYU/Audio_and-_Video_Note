/********************************************************************************
** Form generated from reading UI file 'playlistbox.ui'
**
** Created by: Qt User Interface Compiler version 6.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PLAYLISTBOX_H
#define UI_PLAYLISTBOX_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PlayListBox
{
public:
    QGridLayout *gridLayout;
    QListWidget *List;

    void setupUi(QWidget *PlayListBox)
    {
        if (PlayListBox->objectName().isEmpty())
            PlayListBox->setObjectName("PlayListBox");
        PlayListBox->resize(115, 254);
        gridLayout = new QGridLayout(PlayListBox);
        gridLayout->setSpacing(0);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setContentsMargins(0, 0, 0, 0);
        List = new QListWidget(PlayListBox);
        List->setObjectName("List");

        gridLayout->addWidget(List, 0, 0, 1, 1);


        retranslateUi(PlayListBox);

        QMetaObject::connectSlotsByName(PlayListBox);
    } // setupUi

    void retranslateUi(QWidget *PlayListBox)
    {
        PlayListBox->setWindowTitle(QCoreApplication::translate("PlayListBox", "PlayListBox", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PlayListBox: public Ui_PlayListBox {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PLAYLISTBOX_H
