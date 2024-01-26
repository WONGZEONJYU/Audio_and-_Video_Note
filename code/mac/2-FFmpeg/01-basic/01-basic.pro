TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        avframe.cpp \
        avpacket.cpp \
        main.cpp

INCLUDEPATH += /usr/local/ffmpeg/include

LIBS += -L/usr/local/ffmpeg/lib -lavcodec -lavdevice -lavfilter \
-lavformat -lavutil -lpostproc -lswscale

CONFIG += shadow -build
DESTDIR = $$PWD/bin

HEADERS += \
    avframe.h \
    avpacket.h
