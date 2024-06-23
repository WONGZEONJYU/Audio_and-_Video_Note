/********************************************************************************
** Form generated from reading UI file 'titlebar.ui'
**
** Created by: Qt User Interface Compiler version 6.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TITLEBAR_H
#define UI_TITLEBAR_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TitleBar
{
public:
    QHBoxLayout *horizontalLayout;
    QPushButton *MenuBtn;
    QLabel *MovieNameLab;
    QSpacerItem *horizontalSpacer;
    QPushButton *MinBtn;
    QPushButton *MaxBtn;
    QPushButton *FullScreenBtn;
    QPushButton *CloseBtn;

    void setupUi(QWidget *TitleBar)
    {
        if (TitleBar->objectName().isEmpty())
            TitleBar->setObjectName("TitleBar");
        TitleBar->resize(780, 60);
        TitleBar->setMinimumSize(QSize(0, 60));
        TitleBar->setMaximumSize(QSize(16777215, 60));
        horizontalLayout = new QHBoxLayout(TitleBar);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        MenuBtn = new QPushButton(TitleBar);
        MenuBtn->setObjectName("MenuBtn");
        MenuBtn->setMinimumSize(QSize(160, 50));
        MenuBtn->setMaximumSize(QSize(160, 16777215));
        QFont font;
        font.setFamilies({QString::fromUtf8("Bahnschrift Light SemiCondensed")});
        font.setPointSize(18);
        MenuBtn->setFont(font);

        horizontalLayout->addWidget(MenuBtn);

        MovieNameLab = new QLabel(TitleBar);
        MovieNameLab->setObjectName("MovieNameLab");
        MovieNameLab->setMinimumSize(QSize(420, 50));
        MovieNameLab->setMaximumSize(QSize(16777215, 50));
        MovieNameLab->setWordWrap(false);

        horizontalLayout->addWidget(MovieNameLab);

        horizontalSpacer = new QSpacerItem(45, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        MinBtn = new QPushButton(TitleBar);
        MinBtn->setObjectName("MinBtn");
        MinBtn->setMinimumSize(QSize(50, 50));
        MinBtn->setMaximumSize(QSize(50, 50));

        horizontalLayout->addWidget(MinBtn);

        MaxBtn = new QPushButton(TitleBar);
        MaxBtn->setObjectName("MaxBtn");
        MaxBtn->setMinimumSize(QSize(50, 50));
        MaxBtn->setMaximumSize(QSize(50, 50));

        horizontalLayout->addWidget(MaxBtn);

        FullScreenBtn = new QPushButton(TitleBar);
        FullScreenBtn->setObjectName("FullScreenBtn");
        FullScreenBtn->setMinimumSize(QSize(50, 50));
        FullScreenBtn->setMaximumSize(QSize(50, 50));

        horizontalLayout->addWidget(FullScreenBtn);

        CloseBtn = new QPushButton(TitleBar);
        CloseBtn->setObjectName("CloseBtn");
        CloseBtn->setMinimumSize(QSize(50, 50));
        CloseBtn->setMaximumSize(QSize(50, 50));

        horizontalLayout->addWidget(CloseBtn);


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
