include( ../../gui.pri )
lessThan( QT_VERSION, 5.00 ) {
include( $${SINGLEDIR}/qtsingleapplication.pri )
}

TARGET        = us
TRANSLATIONS += $${TARGET}_de_DE.ts
greaterThan( QT_VERSION, 4.99 ) {
QT           += widgets
QT           += network
}

revision.target           = us_revision.h
revision.commands         = sh revision.sh
revision.depends          = FORCE
unix:QMAKE_EXTRA_TARGETS += revision
mac:ICON                  = ../../etc/us3-icon.icns

HEADERS      += us.h            \
                us_revision.h   \
                us_search.h

SOURCES      += us.cpp          \
                us_win_data.cpp \
                us_search.cpp


