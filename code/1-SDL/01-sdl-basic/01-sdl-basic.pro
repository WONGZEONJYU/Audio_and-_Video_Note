TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

CONFIG += shadow -build
DESTDIR = ../bin

SOURCES += \
        main.cpp

INCLUDEPATH += $$PWD/SDL2-2.28.5-VC/include
LIBS += $$PWD/SDL2-2.28.5-VC/lib/x64/SDL2.lib
