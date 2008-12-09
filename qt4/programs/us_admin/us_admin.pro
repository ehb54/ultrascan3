TEMPLATE      = app

QWTPATH       = /usr/local/qwt-5.1.1

CONFIG       += debug qt thread warn
TARGET        = us_admin
DEPENDPATH   += ../../utils ../../gui
INCLUDEPATH  += ../../utils ../../gui $$QWTPATH/include
DESTDIR       = ../../bin
LIBS         += -lus_utils -lus_gui -L../../../lib -lqca
LIBS         += -lqwt -L$$QWTPATH/lib

HEADERS      += us_admin.h
SOURCES      += us_admin.cpp
TRANSLATIONS += us_admin.ts

DEFINES      += LINUX
