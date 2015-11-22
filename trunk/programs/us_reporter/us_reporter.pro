include( ../../gui.pri )

QT           += xml svg
greaterThan( QT_VERSION, 4.99 ) {
QT           += printsupport
}

TARGET        = us_reporter

HEADERS       = us_reporter.h  \
                us_sync_db.h

SOURCES       = us_reporter.cpp \
                us_sync_db.cpp

