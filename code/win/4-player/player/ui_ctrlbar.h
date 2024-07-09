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
    QGridLayout *gridLayout;
    QPushButton *VolumeBtn;
    QSlider *VolumeSlider;
    QWidget *widget;
    QGridLayout *gridLayout_2;
    QPushButton *PlayOrPauseBtn;
    QPushButton *StopBtn;
    QPushButton *BackwardBtn;
    QPushButton *ForwardBtn;
    QPushButton *SpeedBtn;
    QTimeEdit *VideoPlayTimeTimeEdit;
    QLabel *TimeSplitLabel;
    QTimeEdit *VideoTotalTimeTimeEdit;
    QSpacerItem *horizontalSpacer;
    QPushButton *PlaylistCtrlBtn;
    QPushButton *SettingBtn;

    void setupUi(QWidget *CtrlBar)
    {
        if (CtrlBar->objectName().isEmpty())
            CtrlBar->setObjectName("CtrlBar");
        CtrlBar->resize(594, 60);
        CtrlBar->setMaximumSize(QSize(16777215, 16777215));
        gridLayout_3 = new QGridLayout(CtrlBar);
        gridLayout_3->setSpacing(0);
        gridLayout_3->setObjectName("gridLayout_3");
        gridLayout_3->setContentsMargins(0, 0, 0, 0);
        PlaySliderBgWidget = new QWidget(CtrlBar);
        PlaySliderBgWidget->setObjectName("PlaySliderBgWidget");
        PlaySliderBgWidget->setMinimumSize(QSize(0, 0));
        PlaySliderBgWidget->setMaximumSize(QSize(16777215, 30));
        horizontalLayout = new QHBoxLayout(PlaySliderBgWidget);
        horizontalLayout->setSpacing(1);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(5, 0, 0, 0);
        PlaySlider = new QSlider(PlaySliderBgWidget);
        PlaySlider->setObjectName("PlaySlider");
        PlaySlider->setMinimumSize(QSize(0, 0));
        PlaySlider->setMaximum(65536);
        PlaySlider->setOrientation(Qt::Horizontal);

        horizontalLayout->addWidget(PlaySlider);

        VolumeBgWidget = new QWidget(PlaySliderBgWidget);
        VolumeBgWidget->setObjectName("VolumeBgWidget");
        VolumeBgWidget->setEnabled(true);
        VolumeBgWidget->setMinimumSize(QSize(0, 0));
        VolumeBgWidget->setMaximumSize(QSize(108, 25));
        gridLayout = new QGridLayout(VolumeBgWidget);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setHorizontalSpacing(1);
        gridLayout->setVerticalSpacing(0);
        gridLayout->setContentsMargins(1, 0, 0, 0);
        VolumeBtn = new QPushButton(VolumeBgWidget);
        VolumeBtn->setObjectName("VolumeBtn");
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(VolumeBtn->sizePolicy().hasHeightForWidth());
        VolumeBtn->setSizePolicy(sizePolicy);
        VolumeBtn->setMinimumSize(QSize(20, 20));
        VolumeBtn->setMaximumSize(QSize(16777215, 20));
        VolumeBtn->setMouseTracking(false);

        gridLayout->addWidget(VolumeBtn, 0, 0, 1, 1);

        VolumeSlider = new QSlider(VolumeBgWidget);
        VolumeSlider->setObjectName("VolumeSlider");
        VolumeSlider->setMinimumSize(QSize(88, 20));
        VolumeSlider->setMaximumSize(QSize(16777215, 16777215));
        VolumeSlider->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(VolumeSlider, 0, 1, 1, 1);


        horizontalLayout->addWidget(VolumeBgWidget);


        gridLayout_3->addWidget(PlaySliderBgWidget, 0, 0, 1, 1);

        widget = new QWidget(CtrlBar);
        widget->setObjectName("widget");
        gridLayout_2 = new QGridLayout(widget);
        gridLayout_2->setSpacing(0);
        gridLayout_2->setObjectName("gridLayout_2");
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        PlayOrPauseBtn = new QPushButton(widget);
        PlayOrPauseBtn->setObjectName("PlayOrPauseBtn");
        PlayOrPauseBtn->setMinimumSize(QSize(30, 30));
        PlayOrPauseBtn->setMaximumSize(QSize(30, 30));

        gridLayout_2->addWidget(PlayOrPauseBtn, 0, 0, 1, 1);

        StopBtn = new QPushButton(widget);
        StopBtn->setObjectName("StopBtn");
        StopBtn->setMinimumSize(QSize(30, 30));
        StopBtn->setMaximumSize(QSize(30, 30));

        gridLayout_2->addWidget(StopBtn, 0, 1, 1, 1);

        BackwardBtn = new QPushButton(widget);
        BackwardBtn->setObjectName("BackwardBtn");
        BackwardBtn->setMinimumSize(QSize(30, 30));
        BackwardBtn->setMaximumSize(QSize(30, 30));

        gridLayout_2->addWidget(BackwardBtn, 0, 2, 1, 1);

        ForwardBtn = new QPushButton(widget);
        ForwardBtn->setObjectName("ForwardBtn");
        ForwardBtn->setMinimumSize(QSize(30, 30));
        ForwardBtn->setMaximumSize(QSize(30, 30));

        gridLayout_2->addWidget(ForwardBtn, 0, 3, 1, 1);

        SpeedBtn = new QPushButton(widget);
        SpeedBtn->setObjectName("SpeedBtn");
        SpeedBtn->setMinimumSize(QSize(30, 30));
        SpeedBtn->setMaximumSize(QSize(100, 30));

        gridLayout_2->addWidget(SpeedBtn, 0, 4, 1, 1);

        VideoPlayTimeTimeEdit = new QTimeEdit(widget);
        VideoPlayTimeTimeEdit->setObjectName("VideoPlayTimeTimeEdit");
        VideoPlayTimeTimeEdit->setMaximumSize(QSize(70, 16777215));
        VideoPlayTimeTimeEdit->setMouseTracking(false);
        VideoPlayTimeTimeEdit->setFocusPolicy(Qt::NoFocus);
        VideoPlayTimeTimeEdit->setAcceptDrops(false);
        VideoPlayTimeTimeEdit->setInputMethodHints(Qt::ImhNone);
        VideoPlayTimeTimeEdit->setReadOnly(true);
        VideoPlayTimeTimeEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);
        VideoPlayTimeTimeEdit->setCalendarPopup(false);

        gridLayout_2->addWidget(VideoPlayTimeTimeEdit, 0, 5, 1, 1);

        TimeSplitLabel = new QLabel(widget);
        TimeSplitLabel->setObjectName("TimeSplitLabel");

        gridLayout_2->addWidget(TimeSplitLabel, 0, 6, 1, 1);

        VideoTotalTimeTimeEdit = new QTimeEdit(widget);
        VideoTotalTimeTimeEdit->setObjectName("VideoTotalTimeTimeEdit");
        VideoTotalTimeTimeEdit->setMaximumSize(QSize(70, 16777215));
        VideoTotalTimeTimeEdit->setInputMethodHints(Qt::ImhNone);
        VideoTotalTimeTimeEdit->setReadOnly(true);
        VideoTotalTimeTimeEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);

        gridLayout_2->addWidget(VideoTotalTimeTimeEdit, 0, 7, 1, 1);

        horizontalSpacer = new QSpacerItem(223, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer, 0, 8, 1, 1);

        PlaylistCtrlBtn = new QPushButton(widget);
        PlaylistCtrlBtn->setObjectName("PlaylistCtrlBtn");
        PlaylistCtrlBtn->setMinimumSize(QSize(30, 30));
        PlaylistCtrlBtn->setMaximumSize(QSize(30, 30));

        gridLayout_2->addWidget(PlaylistCtrlBtn, 0, 9, 1, 1);

        SettingBtn = new QPushButton(widget);
        SettingBtn->setObjectName("SettingBtn");
        SettingBtn->setMinimumSize(QSize(30, 30));
        SettingBtn->setMaximumSize(QSize(30, 30));

        gridLayout_2->addWidget(SettingBtn, 0, 10, 1, 1);


        gridLayout_3->addWidget(widget, 1, 0, 1, 1);


        retranslateUi(CtrlBar);

        QMetaObject::connectSlotsByName(CtrlBar);
    } // setupUi

    void retranslateUi(QWidget *CtrlBar)
    {
        CtrlBar->setWindowTitle(QCoreApplication::translate("CtrlBar", "CtrlBar", nullptr));
        VolumeBtn->setText(QString());
        PlayOrPauseBtn->setText(QCoreApplication::translate("CtrlBar", "Play", nullptr));
        StopBtn->setText(QString());
        BackwardBtn->setText(QString());
        ForwardBtn->setText(QString());
        SpeedBtn->setText(QCoreApplication::translate("CtrlBar", "\345\200\215\346\225\2601.0", nullptr));
        VideoPlayTimeTimeEdit->setDisplayFormat(QCoreApplication::translate("CtrlBar", "HH:mm:ss", nullptr));
        TimeSplitLabel->setText(QCoreApplication::translate("CtrlBar", "/", nullptr));
        VideoTotalTimeTimeEdit->setDisplayFormat(QCoreApplication::translate("CtrlBar", "HH:mm:ss", nullptr));
        PlaylistCtrlBtn->setText(QString());
        SettingBtn->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class CtrlBar: public Ui_CtrlBar {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CTRLBAR_H
