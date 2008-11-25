TEMPLATE      = app

CONFIG       += debug qt thread warn
TARGET        = us_config
DEPENDPATH   += ../../utils ../../gui
INCLUDEPATH  += ../../utils ../../gui
DESTDIR       = ../../bin
LIBS         += -lus_utils -lus_gui -L../../../lib -lqca

HEADERS      += us_config.h   \
                us_admin.h    \
                us_database.h \
                us_font.h

SOURCES      += us_config.cpp   \
                us_admin.cpp    \
                us_database.cpp \
                us_font.cpp

TRANSLATIONS += us_config.ts

DEFINES      += LINUX
