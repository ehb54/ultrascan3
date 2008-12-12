TEMPLATE      = app

QWTPATH       = /usr/local/qwt-5.1.1

CONFIG       += debug qt thread warn
TARGET        = us_license
DEPENDPATH   += ../../utils ../../gui
INCLUDEPATH  += ../../utils ../../gui $$QWTPATH/include
DESTDIR       = ../../bin
LIBS         += -lus_utils -lus_gui -L../../../lib -lqca
LIBS         += -lqwt -L$$QWTPATH/lib

SOURCES       = us_license.cpp

