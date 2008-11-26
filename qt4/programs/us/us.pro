include( /opt/qt-4.5.0/addons/qtsingleapplication-2.5_1-commercial/src/qtsingleapplication.pri )
TEMPLATE      = app

CONFIG       += debug qt thread warn
TARGET        = us
DEPENDPATH   += ../../utils ../../gui
INCLUDEPATH  += ../../utils ../../gui
DESTDIR       = ../../bin
LIBS         += -lus_utils -lus_gui -L../../../lib -lqca

HEADERS      += us.h
SOURCES      += us.cpp \
                us_win_data.cpp
TRANSLATIONS += us_de_DE.ts

DEFINES      += LINUX
