TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp

INCLUDEPATH += /usr/local/ffmpeg/include

LIBS += -L/usr/local/ffmpeg/lib -lavcodec -lavdevice -lavfilter \
-lavformat -lavutil -lpostproc -lswscale

CONFIG += shadow -build
DESTDIR = $$PWD/bin

