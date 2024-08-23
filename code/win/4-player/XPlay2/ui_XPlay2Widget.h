/********************************************************************************
** Form generated from reading UI file 'XPlay2Widget.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_XPLAY2WIDGET_H
#define UI_XPLAY2WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <XQSliderWidget.hpp>
#include <XVideoWidget.hpp>

QT_BEGIN_NAMESPACE

class Ui_XPlay2Widget
{
public:
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout;
    XVideoWidget *VideoWidget;
    XQSliderWidget *PlayPos;
    QHBoxLayout *horizontalLayout;
    QPushButton *OpenFile;
    QPushButton *OpenURL;
    QPushButton *isPlay;
    QDoubleSpinBox *Speed;
    QSlider *VolumeSlider;

    void setupUi(QWidget *XPlay2Widget)
    {
        if (XPlay2Widget->objectName().isEmpty())
            XPlay2Widget->setObjectName("XPlay2Widget");
        XPlay2Widget->resize(1287, 848);
        gridLayout = new QGridLayout(XPlay2Widget);
        gridLayout->setSpacing(0);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        VideoWidget = new XVideoWidget(XPlay2Widget);
        VideoWidget->setObjectName("VideoWidget");
        VideoWidget->setMinimumSize(QSize(1280, 760));

        verticalLayout->addWidget(VideoWidget);

        PlayPos = new XQSliderWidget(XPlay2Widget);
        PlayPos->setObjectName("PlayPos");
        PlayPos->setMaximum(999);
        PlayPos->setOrientation(Qt::Horizontal);

        verticalLayout->addWidget(PlayPos);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName("horizontalLayout");
        OpenFile = new QPushButton(XPlay2Widget);
        OpenFile->setObjectName("OpenFile");

        horizontalLayout->addWidget(OpenFile);

        OpenURL = new QPushButton(XPlay2Widget);
        OpenURL->setObjectName("OpenURL");

        horizontalLayout->addWidget(OpenURL);

        isPlay = new QPushButton(XPlay2Widget);
        isPlay->setObjectName("isPlay");

        horizontalLayout->addWidget(isPlay);

        Speed = new QDoubleSpinBox(XPlay2Widget);
        Speed->setObjectName("Speed");
        Speed->setMinimumSize(QSize(100, 0));
        Speed->setMaximumSize(QSize(100, 16777215));
        Speed->setDecimals(1);
        Speed->setMinimum(0.100000000000000);
        Speed->setMaximum(5.000000000000000);
        Speed->setSingleStep(0.100000000000000);
        Speed->setValue(1.000000000000000);

        horizontalLayout->addWidget(Speed);

        VolumeSlider = new QSlider(XPlay2Widget);
        VolumeSlider->setObjectName("VolumeSlider");
        VolumeSlider->setMinimumSize(QSize(0, 0));
        VolumeSlider->setMaximumSize(QSize(100, 16777215));
        VolumeSlider->setMaximum(100);
        VolumeSlider->setOrientation(Qt::Horizontal);

        horizontalLayout->addWidget(VolumeSlider);


        verticalLayout->addLayout(horizontalLayout);


        gridLayout->addLayout(verticalLayout, 0, 0, 1, 1);


        retranslateUi(XPlay2Widget);
        QObject::connect(OpenFile, SIGNAL(clicked()), XPlay2Widget, SLOT(OpenFile()));
        QObject::connect(isPlay, SIGNAL(clicked()), XPlay2Widget, SLOT(PlayOrPause()));
        QObject::connect(PlayPos, SIGNAL(sliderReleased()), XPlay2Widget, SLOT(SliderReleased()));
        QObject::connect(PlayPos, SIGNAL(sliderPressed()), XPlay2Widget, SLOT(SliderPressed()));
        QObject::connect(OpenURL, SIGNAL(clicked()), XPlay2Widget, SLOT(OpenURL()));
        QObject::connect(VolumeSlider, SIGNAL(valueChanged(int)), XPlay2Widget, SLOT(VolumeChanged(int)));
        QObject::connect(Speed, SIGNAL(valueChanged(double)), XPlay2Widget, SLOT(SpeedChanged(double)));

        QMetaObject::connectSlotsByName(XPlay2Widget);
    } // setupUi

    void retranslateUi(QWidget *XPlay2Widget)
    {
        XPlay2Widget->setWindowTitle(QCoreApplication::translate("XPlay2Widget", "XPlay2Widget", nullptr));
        OpenFile->setText(QCoreApplication::translate("XPlay2Widget", "Open Media File", nullptr));
        OpenURL->setText(QCoreApplication::translate("XPlay2Widget", "OpenURL", nullptr));
        isPlay->setText(QCoreApplication::translate("XPlay2Widget", "Play", nullptr));
    } // retranslateUi

};

namespace Ui {
    class XPlay2Widget: public Ui_XPlay2Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_XPLAY2WIDGET_H
