include ( ../library.pri )

QT                 += sql

unix:  TARGET       = us_db

win32 {
       TARGET       = libus_db
       LIBS        += ../lib/libus_utils.lib
}

TRANSLATIONS = $${TARGET}_DE_de.ts

HEADERS      = us_db.h      \
               us_db2.h 

SOURCES      = us_db.cpp    \
               us_db2.cpp

