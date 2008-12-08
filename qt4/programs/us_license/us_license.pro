TEMPLATE      = app

CONFIG       += debug qt thread warn
TARGET        = us_license
DEPENDPATH   += ../../utils ../../gui
INCLUDEPATH  += ../../utils ../../gui
DESTDIR       = ../../bin
LIBS         += -lus_utils -lus_gui -L../../../lib -lqca

SOURCES       = us_license.cpp

