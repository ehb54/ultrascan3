include ( ../library.pri )

!contains( DEFINES, NO_DB ) {
       QT          += sql
       QT          -= gui
}

unix:  TARGET       = us_db

win32 {
       TARGET       = libus_db
       LIBS        += ../lib/libus_utils.lib
}

macx {
       TARGET       = us_db
       LIBS        += -L/usr/local/lib/mysql -lmysqlclient
       LIBS        += -L../lib -lus_utils
}

TRANSLATIONS = $${TARGET}_DE_de.ts

HEADERS      = us_db2.h 

SOURCES      = us_db2.cpp

