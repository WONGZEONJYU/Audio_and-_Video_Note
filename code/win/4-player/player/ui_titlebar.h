/********************************************************************************
** Form generated from reading UI file 'titlebar.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TITLEBAR_H
#define UI_TITLEBAR_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TitleBar
{
public:
    QGridLayout *gridLayout;
    QPushButton *MenuBtn;
    QLabel *MovieNameLab;
    QPushButton *MinBtn;
    QPushButton *MaxBtn;
    QPushButton *FullScreenBtn;
    QPushButton *CloseBtn;

    void setupUi(QWidget *TitleBar)
    {
        if (TitleBar->objectName().isEmpty())
            TitleBar->setObjectName("TitleBar");
        TitleBar->resize(826, 50);
        TitleBar->setMaximumSize(QSize(16777215, 50));
        gridLayout = new QGridLayout(TitleBar);
        gridLayout->setSpacing(0);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setContentsMargins(0, 0, 0, 0);
        MenuBtn = new QPushButton(TitleBar);
        MenuBtn->setObjectName("MenuBtn");
        MenuBtn->setMinimumSize(QSize(200, 33));
        MenuBtn->setMaximumSize(QSize(200, 33));
        QFont font;
        font.setFamilies({QString::fromUtf8("Bahnschrift Light SemiCondensed")});
        font.setPointSize(18);
        MenuBtn->setFont(font);

        gridLayout->addWidget(MenuBtn, 0, 0, 1, 1);

        MovieNameLab = new QLabel(TitleBar);
        MovieNameLab->setObjectName("MovieNameLab");
        MovieNameLab->setMinimumSize(QSize(426, 50));
        MovieNameLab->setMaximumSize(QSize(426, 50));
        MovieNameLab->setWordWrap(false);

        gridLayout->addWidget(MovieNameLab, 0, 1, 1, 1);

        MinBtn = new QPushButton(TitleBar);
        MinBtn->setObjectName("MinBtn");
        MinBtn->setMinimumSize(QSize(50, 50));
        MinBtn->setMaximumSize(QSize(50, 50));

        gridLayout->addWidget(MinBtn, 0, 2, 1, 1);

        MaxBtn = new QPushButton(TitleBar);
        MaxBtn->setObjectName("MaxBtn");
        MaxBtn->setMinimumSize(QSize(50, 50));
        MaxBtn->setMaximumSize(QSize(50, 50));

        gridLayout->addWidget(MaxBtn, 0, 3, 1, 1);

        FullScreenBtn = new QPushButton(TitleBar);
        FullScreenBtn->setObjectName("FullScreenBtn");
        FullScreenBtn->setMinimumSize(QSize(50, 50));
        FullScreenBtn->setMaximumSize(QSize(50, 50));

        gridLayout->addWidget(FullScreenBtn, 0, 4, 1, 1);

        CloseBtn = new QPushButton(TitleBar);
        CloseBtn->setObjectName("CloseBtn");
        CloseBtn->setMinimumSize(QSize(50, 50));
        CloseBtn->setMaximumSize(QSize(50, 50));

        gridLayout->addWidget(CloseBtn, 0, 5, 1, 1);


        retranslateUi(TitleBar);

        QMetaObject::connectSlotsByName(TitleBar);
    } // setupUi

    void retranslateUi(QWidget *TitleBar)
    {
        TitleBar->setWindowTitle(QCoreApplication::translate("TitleBar", "TitleBar", nullptr));
        MenuBtn->setText(QCoreApplication::translate("TitleBar", "Player", nullptr));
        MovieNameLab->setText(QCoreApplication::translate("TitleBar", "Movie_name", nullptr));
        MinBtn->setText(QString());
        MaxBtn->setText(QString());
        FullScreenBtn->setText(QString());
        CloseBtn->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class TitleBar: public Ui_TitleBar {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TITLEBAR_H
