TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp

INCLUDEPATH += /opt/homebrew/Cellar/sdl2/2.28.5/include
LIBS += -L/opt/homebrew/Cellar/sdl2/2.28.5/lib/ -lSDL2

CONFIG += shadow -build
DESTDIR = $$PWD/bin
