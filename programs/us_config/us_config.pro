TEMPLATE      = app

QWTPATH       = /usr/local/qwt-5.1.1

CONFIG       += debug qt thread warn
TARGET        = us_config
DEPENDPATH   += ../../utils ../../gui ../../db
INCLUDEPATH  += ../../utils ../../gui ../../db $$QWTPATH/include
DESTDIR       = ../../bin
LIBS         += -lus_utils -lus_gui -lus_db -L../../lib -lqca 
LIBS         += -lqwt -L$$QWTPATH/lib

# Temp until db library is built 
QT           += sql svg

HEADERS      += us_config.h   \
                us_admin.h    \
                us_color.h    \
                us_database.h \
                us_font.h

SOURCES      += us_config.cpp   \
                us_admin.cpp    \
                us_color.cpp    \
                us_database.cpp \
                us_font.cpp

TRANSLATIONS += us_config.ts

DEFINES      += LINUX
