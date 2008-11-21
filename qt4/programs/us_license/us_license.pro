TEMPLATE      = app

CONFIG       += debug qt thread warn
TARGET        = us_license
DEPENDPATH   += ../utils ../gui
INCLUDEPATH  += ../utils ../gui
DESTDIR       = .
LIBS         += -lus_utils -lus_gui -L../../lib

SOURCES       = us_license.cpp

