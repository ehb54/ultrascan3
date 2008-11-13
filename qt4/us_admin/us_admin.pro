TEMPLATE      = app

CONFIG       += debug qt thread warn
TARGET        = us_admin
DEPENDPATH   += ../utils ../gui
INCLUDEPATH  += ../utils ../gui
DESTDIR       = ../bin
LIBS         += -lus_utils -lus_gui -L../../lib

HEADERS      += us_admin.h
SOURCES      += us_admin.cpp
TRANSLATIONS += us_admin.ts

DEFINES      += LINUX
