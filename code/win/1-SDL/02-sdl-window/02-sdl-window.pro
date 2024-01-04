TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp

INCLUDEPATH += ../SDL2-2.28.5-VC/include
LIBS += ../SDL2-2.28.5-VC/lib/x64/SDL2.lib

CONFIG += shadow -build
DESTDIR = $$PWD/bin
