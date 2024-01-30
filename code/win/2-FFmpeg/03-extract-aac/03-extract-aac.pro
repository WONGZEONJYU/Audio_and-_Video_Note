TEMPLATE = app
CONFIG += console c++20
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp

INCLUDEPATH += $$PWD/../ffmepg-6.1/include

LIBS += $$PWD/../ffmepg-6.1/bin/avformat.lib \
        $$PWD/../ffmepg-6.1/bin/avcodec.lib \
        $$PWD/../ffmepg-6.1/bin/avdevice.lib \
        $$PWD/../ffmepg-6.1/bin/avfilter.lib \
        $$PWD/../ffmepg-6.1/bin/avutil.lib \
        $$PWD/../ffmepg-6.1/bin/swresample.lib \
        $$PWD/../ffmepg-6.1/bin/swscale.lib

CONFIG += shadow -build
DESTDIR = $$PWD/bin
