/********************************************************************************
** Form generated from reading UI file 'ctrlbar.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CTRLBAR_H
#define UI_CTRLBAR_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTimeEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CtrlBar
{
public:
    QGridLayout *gridLayout_3;
    QWidget *PlaySliderBgWidget;
    QHBoxLayout *horizontalLayout;
    QSlider *PlaySlider;
    QWidget *VolumeBgWidget;
    QGridLayout *gridLayout_2;
    QPushButton *VolumeBtn;
    QSlider *VolumeSlider;
    QGridLayout *gridLayout;
    QPushButton *ForwardBtn;
    QPushButton *SettingBtn;
    QPushButton *StopBtn;
    QPushButton *BackwardBtn;
    QPushButton *SpeedBtn;
    QPushButton *PlaylistCtrlBtn;
    QTimeEdit *VideoTotalTimeTimeEdit;
    QTimeEdit *VideoPlayTimeTimeEdit;
    QLabel *TimeSplitLabel;
    QPushButton *PlayOrPauseBtn;
    QSpacerItem *horizontalSpacer;

    void setupUi(QWidget *CtrlBar)
    {
        if (CtrlBar->objectName().isEmpty())
            CtrlBar->setObjectName("CtrlBar");
        CtrlBar->resize(482, 57);
        gridLayout_3 = new QGridLayout(CtrlBar);
        gridLayout_3->setSpacing(0);
        gridLayout_3->setObjectName("gridLayout_3");
        gridLayout_3->setContentsMargins(0, 0, 0, 0);
        PlaySliderBgWidget = new QWidget(CtrlBar);
        PlaySliderBgWidget->setObjectName("PlaySliderBgWidget");
        PlaySliderBgWidget->setMaximumSize(QSize(16777215, 25));
        horizontalLayout = new QHBoxLayout(PlaySliderBgWidget);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(3, 0, 0, 0);
        PlaySlider = new QSlider(PlaySliderBgWidget);
        PlaySlider->setObjectName("PlaySlider");
        PlaySlider->setMaximum(65536);
        PlaySlider->setOrientation(Qt::Horizontal);

        horizontalLayout->addWidget(PlaySlider);

        VolumeBgWidget = new QWidget(PlaySliderBgWidget);
        VolumeBgWidget->setObjectName("VolumeBgWidget");
        VolumeBgWidget->setEnabled(true);
        VolumeBgWidget->setMaximumSize(QSize(108, 25));
        gridLayout_2 = new QGridLayout(VolumeBgWidget);
        gridLayout_2->setSpacing(0);
        gridLayout_2->setObjectName("gridLayout_2");
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        VolumeBtn = new QPushButton(VolumeBgWidget);
        VolumeBtn->setObjectName("VolumeBtn");
        VolumeBtn->setMinimumSize(QSize(20, 20));
        VolumeBtn->setMaximumSize(QSize(20, 20));

        gridLayout_2->addWidget(VolumeBtn, 0, 0, 1, 1);

        VolumeSlider = new QSlider(VolumeBgWidget);
        VolumeSlider->setObjectName("VolumeSlider");
        VolumeSlider->setMinimumSize(QSize(80, 25));
        VolumeSlider->setMaximumSize(QSize(80, 25));
        VolumeSlider->setOrientation(Qt::Horizontal);

        gridLayout_2->addWidget(VolumeSlider, 0, 1, 1, 1);


        horizontalLayout->addWidget(VolumeBgWidget);


        gridLayout_3->addWidget(PlaySliderBgWidget, 0, 0, 1, 1);

        gridLayout = new QGridLayout();
        gridLayout->setSpacing(0);
        gridLayout->setObjectName("gridLayout");
        ForwardBtn = new QPushButton(CtrlBar);
        ForwardBtn->setObjectName("ForwardBtn");
        ForwardBtn->setMinimumSize(QSize(30, 30));
        ForwardBtn->setMaximumSize(QSize(30, 30));

        gridLayout->addWidget(ForwardBtn, 0, 5, 1, 1);

        SettingBtn = new QPushButton(CtrlBar);
        SettingBtn->setObjectName("SettingBtn");
        SettingBtn->setMinimumSize(QSize(30, 30));
        SettingBtn->setMaximumSize(QSize(30, 30));

        gridLayout->addWidget(SettingBtn, 0, 13, 1, 1);

        StopBtn = new QPushButton(CtrlBar);
        StopBtn->setObjectName("StopBtn");
        StopBtn->setMinimumSize(QSize(30, 30));
        StopBtn->setMaximumSize(QSize(30, 30));

        gridLayout->addWidget(StopBtn, 0, 1, 1, 1);

        BackwardBtn = new QPushButton(CtrlBar);
        BackwardBtn->setObjectName("BackwardBtn");
        BackwardBtn->setMinimumSize(QSize(30, 30));
        BackwardBtn->setMaximumSize(QSize(30, 30));

        gridLayout->addWidget(BackwardBtn, 0, 2, 1, 1);

        SpeedBtn = new QPushButton(CtrlBar);
        SpeedBtn->setObjectName("SpeedBtn");
        SpeedBtn->setMinimumSize(QSize(30, 30));
        SpeedBtn->setMaximumSize(QSize(100, 100));

        gridLayout->addWidget(SpeedBtn, 0, 6, 1, 1);

        PlaylistCtrlBtn = new QPushButton(CtrlBar);
        PlaylistCtrlBtn->setObjectName("PlaylistCtrlBtn");
        PlaylistCtrlBtn->setMinimumSize(QSize(30, 30));
        PlaylistCtrlBtn->setMaximumSize(QSize(30, 30));

        gridLayout->addWidget(PlaylistCtrlBtn, 0, 12, 1, 1);

        VideoTotalTimeTimeEdit = new QTimeEdit(CtrlBar);
        VideoTotalTimeTimeEdit->setObjectName("VideoTotalTimeTimeEdit");
        VideoTotalTimeTimeEdit->setMaximumSize(QSize(70, 16777215));
        VideoTotalTimeTimeEdit->setInputMethodHints(Qt::ImhPreferNumbers);
        VideoTotalTimeTimeEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        VideoTotalTimeTimeEdit->setReadOnly(true);
        VideoTotalTimeTimeEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);

        gridLayout->addWidget(VideoTotalTimeTimeEdit, 0, 9, 1, 1);

        VideoPlayTimeTimeEdit = new QTimeEdit(CtrlBar);
        VideoPlayTimeTimeEdit->setObjectName("VideoPlayTimeTimeEdit");
        VideoPlayTimeTimeEdit->setMaximumSize(QSize(70, 16777215));
        VideoPlayTimeTimeEdit->setInputMethodHints(Qt::ImhPreferNumbers);
        VideoPlayTimeTimeEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        VideoPlayTimeTimeEdit->setReadOnly(true);
        VideoPlayTimeTimeEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);

        gridLayout->addWidget(VideoPlayTimeTimeEdit, 0, 7, 1, 1);

        TimeSplitLabel = new QLabel(CtrlBar);
        TimeSplitLabel->setObjectName("TimeSplitLabel");

        gridLayout->addWidget(TimeSplitLabel, 0, 8, 1, 1);

        PlayOrPauseBtn = new QPushButton(CtrlBar);
        PlayOrPauseBtn->setObjectName("PlayOrPauseBtn");
        PlayOrPauseBtn->setMinimumSize(QSize(30, 30));
        PlayOrPauseBtn->setMaximumSize(QSize(30, 30));

        gridLayout->addWidget(PlayOrPauseBtn, 0, 0, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer, 0, 11, 1, 1);


        gridLayout_3->addLayout(gridLayout, 1, 0, 1, 1);


        retranslateUi(CtrlBar);

        QMetaObject::connectSlotsByName(CtrlBar);
    } // setupUi

    void retranslateUi(QWidget *CtrlBar)
    {
        CtrlBar->setWindowTitle(QCoreApplication::translate("CtrlBar", "CtrlBar", nullptr));
        VolumeBtn->setText(QString());
        ForwardBtn->setText(QString());
        SettingBtn->setText(QString());
        StopBtn->setText(QString());
        BackwardBtn->setText(QString());
        SpeedBtn->setText(QCoreApplication::translate("CtrlBar", "\345\200\215\346\225\2601.0", nullptr));
        PlaylistCtrlBtn->setText(QString());
        VideoTotalTimeTimeEdit->setDisplayFormat(QCoreApplication::translate("CtrlBar", "H:mm:ss", nullptr));
        VideoPlayTimeTimeEdit->setDisplayFormat(QCoreApplication::translate("CtrlBar", "H:mm:ss", nullptr));
        TimeSplitLabel->setText(QCoreApplication::translate("CtrlBar", "/", nullptr));
        PlayOrPauseBtn->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class CtrlBar: public Ui_CtrlBar {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CTRLBAR_H
