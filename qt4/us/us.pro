include( /opt/qt-4.5.0/addons/qtsingleapplication-2.5_1-commercial/src/qtsingleapplication.pri )
TEMPLATE      = app

CONFIG       += debug qt thread warn
TARGET        = us
DEPENDPATH   += ../utils ../gui
INCLUDEPATH  += ../utils ../gui
DESTDIR       = ../bin
LIBS         += -lus_utils -lus_gui -L../../lib

HEADERS      += us.h
SOURCES      += us.cpp
TRANSLATIONS += us.ts

DEFINES      += LINUX
